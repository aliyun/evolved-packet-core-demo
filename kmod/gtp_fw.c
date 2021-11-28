/*****************************************************************
 * File Name: gtp_fw.c
 * Create Date: 2019/04/24
 * Auth: Moan
 * Description:
 * Revision History:
 * Others:
 *  /proc/sys/net/ipv4/conf/[INTF]/accept_local must be enabled
 *  net.ipv4.ip_nonlocal_bind=1
 *  net.ipv4.conf.all.log_martians=1
 *  net.ipv4.conf.all.rp_filter=0
 *  net.ipv4.conf.[INTF].rp_filter=0
 *  net.ipv4.conf.[INTF].accept_local=1
 *****************************************************************/
#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/ip.h>
#include <net/ip.h>
//#include <net/gtp.h>
#include <net/udp.h>
#include <linux/netdevice.h>
#include <net/route.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/rculist.h>
#include <linux/jhash.h>
#include "gtp.h"
#include "gtp_fw.h"


#if !IS_ENABLED(CONFIG_NF_NAT)
make sure CONFIG_NF_NAT is enabled.
#endif
//#define GTP_TEST 1
//#define FORCE_GW_DEV_CHECK 1

//#define ETH_HDR_LEN 14
#define GTP_HDR_LEN 8
#define UDP_HDR_LEN 8
#define UDP_GTP_HDR_LEN 16
#define IP_HDR_LEN 20
#define IP_UDP_HDR_LEN 28
#define IP_UDP_GTP_HDR_LEN 36

u32 g_needed_headroom = LL_MAX_HEADER + IP_UDP_GTP_HDR_LEN;
#ifdef FORCE_GW_DEV_CHECK
static struct net_device* g_gtp_dev = NULL;
#endif

static int send_usrmsg(struct nlmsghdr *req, u8 *pbuf, uint16_t len);
/*****************************************************************
 * GTP FLOW START
 *****************************************************************/
#define GTP_FL_HASH_SIZE 512

struct gtp_fl_node {
    struct hlist_node	hlist_tid;
    struct hlist_node	hlist_addr;
    struct gtp_flow flow;
    struct rcu_head		rcu_head;
};

struct gtp_ctx {
    unsigned int		hash_size;
    struct hlist_head	*tid_hash;
    struct hlist_head	*addr_hash;
};

struct gtp_ctx g_ctx;

static u32 gtp_h_initval;

static inline u32 gtp1u_hashfn(u32 tid)
{
    return jhash_1word(tid, gtp_h_initval);
}

static inline u32 ipv4_hashfn(__be32 ip)
{
    return jhash_1word((__force u32)ip, gtp_h_initval);
}

static int ipv4_flow_add(struct gtp_ctx *ctx, struct gtp_flow *flow)
{
    u32 hash_ms, hash_tid = 0;
    struct gtp_fl_node *fl_node;
    bool found = false;

    hash_ms = ipv4_hashfn(flow->inner_ip) % ctx->hash_size;

    hlist_for_each_entry_rcu(fl_node, &ctx->addr_hash[hash_ms], hlist_addr) {
        if (fl_node->flow.inner_ip == flow->inner_ip) {
            found = true;
            break;
        }
    }

    if (found) {
        memcpy(&(fl_node->flow), flow, sizeof(struct gtp_flow));
        return 0;
    }

    fl_node = kmalloc(sizeof(struct gtp_fl_node), GFP_KERNEL);
    if (fl_node == NULL)
        return -ENOMEM;

    memcpy(&(fl_node->flow), flow, sizeof(struct gtp_flow));
    fl_node->flow.encap_bytes = 0;
    fl_node->flow.decap_bytes = 0;
    fl_node->flow.flags = 0;

    hash_tid = gtp1u_hashfn(fl_node->flow.local_teid) % ctx->hash_size;

    hlist_add_head_rcu(&fl_node->hlist_addr, &ctx->addr_hash[hash_ms]);
    hlist_add_head_rcu(&fl_node->hlist_tid, &ctx->tid_hash[hash_tid]);

    return 0;
}

static void gtp_fl_node_free(struct rcu_head *head)
{
    struct gtp_fl_node *fl_node = container_of(head, struct gtp_fl_node, rcu_head);
    kfree(fl_node);
}

static void gtp_fl_node_delete(struct gtp_fl_node *fl_node)
{
    hlist_del_rcu(&fl_node->hlist_tid);
    hlist_del_rcu(&fl_node->hlist_addr);
    call_rcu(&fl_node->rcu_head, gtp_fl_node_free);
}

static int gtp_hashtable_init(struct gtp_ctx *ctx, int hsize)
{
    int i;

    ctx->addr_hash = kmalloc_array(hsize, sizeof(struct hlist_head),
            GFP_KERNEL);
    if (ctx->addr_hash == NULL)
        return -ENOMEM;

    ctx->tid_hash = kmalloc_array(hsize, sizeof(struct hlist_head),
            GFP_KERNEL);
    if (ctx->tid_hash == NULL)
        goto err1;

    ctx->hash_size = hsize;

    for (i = 0; i < hsize; i++) {
        INIT_HLIST_HEAD(&ctx->addr_hash[i]);
        INIT_HLIST_HEAD(&ctx->tid_hash[i]);
    }
    return 0;
err1:
    kfree(ctx->addr_hash);
    return -ENOMEM;
}

static void gtp_hashtable_free(struct gtp_ctx *ctx)
{
    struct gtp_fl_node *fl_node;
    int i;

    for (i = 0; i < ctx->hash_size; i++)
        hlist_for_each_entry_rcu(fl_node, &ctx->tid_hash[i], hlist_tid)
            gtp_fl_node_delete(fl_node);

    synchronize_rcu();
    kfree(ctx->addr_hash);
    kfree(ctx->tid_hash);
}

static struct gtp_fl_node *gtp_fl_find_by_ul_info(struct gtp_ctx *ctx, u32 dst_ip, u32 tid)
{
    struct hlist_head *head;
    struct gtp_fl_node *fl_node;

    head = &ctx->tid_hash[gtp1u_hashfn(tid) % ctx->hash_size];

    hlist_for_each_entry_rcu(fl_node, head, hlist_tid) {
        if (fl_node->flow.local_teid == tid && fl_node->flow.local_ip == dst_ip) {
            return fl_node;
        }
    }

    return NULL;
}

static struct gtp_fl_node *gtp_fl_find_by_inner_ip(struct gtp_ctx *ctx, __be32 ip_addr)
{
    struct hlist_head *head;
    struct gtp_fl_node *fl_node;

    head = &ctx->addr_hash[ipv4_hashfn(ip_addr) % ctx->hash_size];

    hlist_for_each_entry_rcu(fl_node, head, hlist_addr) {
        if (fl_node->flow.inner_ip == ip_addr)
            return fl_node;
    }

    return NULL;
}

static int gtp_add_flow(struct gtp_flow *flow)
{
    int ret=0;

    rcu_read_lock();
    ret = ipv4_flow_add(&g_ctx, flow);
    rcu_read_unlock();
    if (ret != 0) {
        printk("add gtp flow for ip 0x%x, local_teid=%d not found!", flow->inner_ip, flow->local_teid);
    }

    return ret;
}

static int gtp_del_flow(struct gtp_flow *flow)
{
    struct gtp_fl_node *fl_node = NULL;

    rcu_read_lock();
    //fl_node = gtp_fl_find_by_ul_info(&g_ctx, flow->local_ip, flow->local_teid);
    fl_node = gtp_fl_find_by_inner_ip(&g_ctx, flow->inner_ip);
    if (fl_node == NULL) {
        rcu_read_unlock();
        printk("gtp flow for ip 0x%x, local_teid=%d not found!", flow->inner_ip, flow->local_teid);
        return -1;
    }

    gtp_fl_node_delete(fl_node);
    rcu_read_unlock();

    return 0;
}

static int dump_all_flow(void)
{
    struct gtp_fl_node *fl_node;
    struct gtp_flow *flow;
    int i;

    printk("dump start");
    rcu_read_lock();
    for (i = 0; i < g_ctx.hash_size; i++)
        hlist_for_each_entry_rcu(fl_node, &g_ctx.tid_hash[i], hlist_tid) {
            flow = &fl_node->flow;
            printk("%x:%d(%d,%x)-%x:%d(%d)", flow->remote_ip,
                    flow->remote_port,
                    flow->remote_teid,
                    flow->inner_ip,
                    flow->local_ip,
                    flow->local_port,
                    flow->local_teid);
        }

    rcu_read_unlock();
    printk("dump end");
    return 0;
}

static int send_all_flow(struct nlmsghdr *resp)
{
    struct gtp_fl_node *fl_node;
    struct gtp_flow *flow;
    struct gtp_flow flows[MAX_FLOW_CNT_IN_MSG];
    int i,j=0;

    memset(&flows, 0, sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG);

    resp->nlmsg_type = GTP_MSG_GET_ALL_FLOW_RESP;
    rcu_read_lock();
    for (i=0; i<g_ctx.hash_size; i++)
        hlist_for_each_entry_rcu(fl_node, &g_ctx.tid_hash[i], hlist_tid) {
            flow = &fl_node->flow;
            memcpy(&flows[j++], flow, sizeof(struct gtp_flow));
            if (j==MAX_FLOW_CNT_IN_MSG) {
                send_usrmsg(resp, (u8 *)&flows, sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG);
                memset(&flows, 0, sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG);
                j = 0;
            }
        }

    rcu_read_unlock();

    send_usrmsg(resp, (u8 *)&flows, sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG);
    return 0;
}

int gtp_fl_ctx_init(void)
{
    int ret=0;
    get_random_bytes(&gtp_h_initval, sizeof(gtp_h_initval));
    ret = gtp_hashtable_init(&g_ctx, GTP_FL_HASH_SIZE);
    if (ret != 0) {
        printk("gtp_hashtable_init failed! ret=%d", ret);
    }

    return ret;
}
/*****************************************************************
 * GTP FLOW END
 *****************************************************************/


/*****************************************************************
 * NETLINK CONFIG INTERFACE START
 *****************************************************************/
#define NETLINK_GTP_FW 30

struct sock *g_nlsk = NULL;
extern struct net init_net;

static int send_usrmsg(struct nlmsghdr *req, u8 *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;
    int ret;

    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if (!nl_skb) {
        printk("netlink alloc failure\n");
        return -1;
    }

    nlh = nlmsg_put(nl_skb, req->nlmsg_pid, req->nlmsg_seq, req->nlmsg_type, len, 0);
    if (nlh == NULL) {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }

    if (len > 0) {
        memcpy(nlmsg_data(nlh), pbuf, len);
    }
    ret = netlink_unicast(g_nlsk, nl_skb, req->nlmsg_pid, MSG_DONTWAIT);

    return ret;
}

static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    struct gtp_flow *flow = NULL;
    struct nlmsghdr resp;
    u32 ret=0;

    if (skb->len <= nlmsg_total_size(0)) {
        return;
    }

    nlh = nlmsg_hdr(skb);
    if (!nlh) {
        printk("%s %d: nlh is NULL!", __FUNCTION__, __LINE__);
    }

    printk("nlh->nlmsg_type=%d, nlh->nlmsg_len=%d", nlh->nlmsg_type, nlh->nlmsg_len);
    memcpy(&resp, nlh, sizeof(struct nlmsghdr));
    resp.nlmsg_type = GTP_MSG_RESP_FAILED;

    if (nlh->nlmsg_len < sizeof(struct nlmsghdr)) {
        send_usrmsg(nlh, NULL, 0);
        return;
    }

    //dump_all_flow();

    switch(nlh->nlmsg_type) {
        case GTP_MSG_ADD_FLOW:
            if (nlh->nlmsg_len != (sizeof(struct nlmsghdr) + sizeof(struct gtp_flow))) {
                send_usrmsg(nlh, NULL, 0);
                break;
            }
            flow = (struct gtp_flow*)NLMSG_DATA(nlh);
            //printk("flow: %x-%d-%d", flow->inner_ip, flow->local_teid, flow->remote_teid);
            ret = gtp_add_flow(flow);
            if (ret == 0) {
                resp.nlmsg_type = GTP_MSG_RESP_OK;
            }
            send_usrmsg(&resp, NULL, 0);
            break;

        case GTP_MSG_DEL_FLOW:
            if (nlh->nlmsg_len != (sizeof(struct nlmsghdr) + sizeof(struct gtp_flow))) {
                send_usrmsg(nlh, NULL, 0);
                break;
            }
            flow = (struct gtp_flow*)NLMSG_DATA(nlh);
            //printk("flow: %x-%d-%d", flow->inner_ip, flow->local_teid, flow->remote_teid);
            ret = gtp_del_flow(flow);
            if (ret == 0) {
                resp.nlmsg_type = GTP_MSG_RESP_OK;
            }
            send_usrmsg(&resp, NULL, 0);
            break;

        case GTP_MSG_DUMP_FLOW:
            ret=dump_all_flow();
            if (ret == 0) {
                resp.nlmsg_type = GTP_MSG_RESP_OK;
            }
            send_usrmsg(&resp, NULL, 0);
            break;

        case GTP_MSG_GET_ALL_FLOW:
            send_all_flow(&resp);
            break;

        default:
            printk("%s %d: unsupported msg %d", __FUNCTION__, __LINE__, nlh->nlmsg_type);
            //ret = -1;
    }
}

struct netlink_kernel_cfg gtp_nl_cfg = { 
    .input  = netlink_rcv_msg,
};

int gtp_fw__netlink_init(void)
{
    /* create netlink socket */
    g_nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_GTP_FW, &gtp_nl_cfg);
    if(g_nlsk == NULL)
    {   
        printk("gtp_fw__netlink_init error !\n");
        return -1; 
    }   

    return 0;
}

void gtp_fw_netlink_exit(void)
{
    if (g_nlsk){
        netlink_kernel_release(g_nlsk);
        g_nlsk = NULL;
    }   
}
/*****************************************************************
 * NETLINK CONFIG INTERFACE END
 *****************************************************************/

#if 0
static void print_skb_t(struct sk_buff *skb) {
    int i=0;
    u8 buf[2048] = {0};
    u8 tmp[10] = {0};
    int len=200;

    sprintf(buf, "%s %d-", skb->dev->name, skb->len);

    if (skb->len < len) {
        len = skb->len;
    }

    for (i=0; i<len; i++) {
        sprintf(tmp, "%02x ", skb->data[i]);
        strcat(buf, tmp);
    }
    printk(buf);
}
#endif

#define GTPU_FAILURE 1
#define GTPU_SUCCESS !GTPU_FAILURE

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
static unsigned int gtp_fw_process_pre_uplink(void *priv,
        struct sk_buff *skb,
        const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static unsigned int gtp_fw_process_pre_uplink(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
#else
#endif
{
    unsigned int proto=0;
    struct iphdr *iph=NULL;
    __be32 daddr=0;
    struct udphdr *uh=NULL;
    struct gtp1_header *gtp1=NULL;
    struct gtp_fl_node *fl_node=NULL;

    if (!skb) {
        return NF_DROP;
    }

    proto = ntohs(skb->protocol);
    if (proto != ETH_P_IP) {
        return NF_ACCEPT;
    }

    iph = ip_hdr(skb);
    if (!iph) {
        return NF_DROP;
    }
    daddr = iph->daddr;

    if (iph->protocol != IPPROTO_UDP) {
        return NF_ACCEPT;
    }

    uh = udp_hdr(skb);
    if (uh->dest != htons(GTP1U_PORT)) {
        return NF_ACCEPT;
    }

    gtp1 = (struct gtp1_header *)((char *)uh + sizeof(struct gtp1_header));
    if (gtp1->type != GTP_TPDU) {
        return NF_ACCEPT;
    }

    /* check dst_ip and teid */
    rcu_read_lock();
    fl_node = gtp_fl_find_by_ul_info(&g_ctx, ntohl(daddr), ntohl(gtp1->tid));
    rcu_read_unlock();
    if (fl_node == NULL) {
        return NF_ACCEPT;
    }

    iph = (struct iphdr *)(skb->data + IP_UDP_GTP_HDR_LEN);
    if (iph->protocol != IPPROTO_ICMP && iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP) {
        return NF_ACCEPT;
    }
   
    if (!pskb_may_pull(skb, IP_UDP_GTP_HDR_LEN+IP_UDP_HDR_LEN)) {
	return NF_ACCEPT;
    }

    pskb_pull(skb, IP_UDP_GTP_HDR_LEN);
    skb->mac_header += IP_UDP_GTP_HDR_LEN;
    skb->network_header += IP_UDP_GTP_HDR_LEN;
    skb->transport_header += IP_UDP_GTP_HDR_LEN;
    //skb_set_network_header(skb, 0);
    //skb_set_transport_header(skb, IP_HDR_LEN);
#ifdef FORCE_GW_DEV_CHECK
    skb->dev = g_gtp_dev;
#endif
    skb->csum = skb_checksum(skb, IP_HDR_LEN, skb->len-IP_HDR_LEN, 0);

    fl_node->flow.decap_bytes += skb->len;

    return NF_ACCEPT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
static unsigned int gtp_fw_process_pre_downlink(void *priv,
        struct sk_buff *skb,
        const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static unsigned int gtp_fw_process_pre_downlink(unsigned int hooknum,
        struct sk_buff *skb,
        const struct net_device *in,
        const struct net_device *out,
        int (*okfn)(struct sk_buff *))
#else
#endif
{
    unsigned int proto=0;
    struct iphdr *iph=NULL;
    __be32 daddr=0;
    struct udphdr *uh=NULL;
    struct gtp1_header *gtp1=NULL;
    struct net *net=NULL;
    u8 tos=0, ttl=64;
    struct gtp_flow* flow = NULL;
    struct gtp_fl_node *fl_node=NULL;

    if (!skb) {
        return NF_DROP;
    }
    net = dev_net(skb->dev);

    proto = ntohs(skb->protocol);
    if (proto != ETH_P_IP) {
        return NF_ACCEPT;
    }

    iph = ip_hdr(skb);
    if (!iph) {
        return NF_DROP;
    }
    daddr = iph->daddr;

    /* check dst_ip and teid */
    rcu_read_lock();
    fl_node = gtp_fl_find_by_inner_ip(&g_ctx, ntohl(daddr));
    rcu_read_unlock();
    if (fl_node == NULL) {
        return NF_ACCEPT;
    }

    tos = iph->tos;
    ttl = iph->ttl - 1;

    flow = &(fl_node->flow);

    /* encap */
    /* Ensure there is sufficient headroom. */
    if (skb_cow_head(skb, g_needed_headroom))
        return NF_ACCEPT;

    skb_push(skb, UDP_GTP_HDR_LEN);
    skb->transport_header -= IP_UDP_GTP_HDR_LEN;

    gtp1 = (struct gtp1_header *)(skb->data + UDP_HDR_LEN);
    gtp1->flags	= 0x30;
    gtp1->type	= GTP_TPDU;
    gtp1->length	= htons(skb->len - UDP_GTP_HDR_LEN);
    gtp1->tid	= htonl(flow->remote_teid);

    uh = udp_hdr(skb);
    uh->dest = htons(flow->remote_port);
    uh->source = htons(flow->local_port);
    uh->len = htons(skb->len);
    udp_set_csum(false, skb, htonl(flow->local_ip), htonl(flow->remote_ip), skb->len);

    skb_push(skb, IP_HDR_LEN);
    skb->mac_header -= IP_UDP_GTP_HDR_LEN;
    skb->network_header -= IP_UDP_GTP_HDR_LEN;

    iph = ip_hdr(skb);
    iph->version = 4;
    iph->ihl = sizeof(struct iphdr) >> 2;
    iph->frag_off = 0;
    iph->protocol = IPPROTO_UDP;
    iph->tos = tos;
    iph->ttl = ttl;
    iph->daddr = htonl(flow->remote_ip);
    iph->saddr = htonl(flow->local_ip);

    iph->tot_len = htons(skb->len);
    __ip_select_ident(net, iph, skb_shinfo(skb)->gso_segs ?: 1);    
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

    flow->encap_bytes += skb->len;

    return NF_ACCEPT;
}

#ifdef GTP_TEST
u32 test_count=0;
static unsigned int gtp_fw_process_test(void *priv,
        struct sk_buff *skb,
        const struct nf_hook_state *state)
{
    //struct gtp_flow* flow = NULL;
    struct iphdr *iph=ip_hdr(skb);
    __be32 daddr = iph->daddr;
    unsigned int proto = ntohs(skb->protocol);
    //struct gtp1_header *gtp1;
    //struct udphdr *uh;
    //struct net *net = dev_net(skb->dev);
    //u8 tos=0, ttl=64;
    struct gtp_fl_node *fl_node = NULL;
    //u32 csum = 0;


    if (proto != ETH_P_IP) {
        return NF_ACCEPT;
    }

    /* check dst_ip and teid */
    rcu_read_lock();
    fl_node = gtp_fl_find_by_inner_ip(&g_ctx, ntohl(daddr));
    rcu_read_unlock();
    if (fl_node == NULL) {
        if (ntohl(daddr) == 0x28282802 || ntohl(daddr) == 0x28282803 ||
                ntohl(daddr) == 0x32323202 || ntohl(daddr) == 0x32323203) {
            int i=0;
            char buf1[1024], buf2[1024]={0};
            ip_fast_csum((unsigned char *)iph, iph->ihl);
            skb_checksum(skb, IP_HDR_LEN, skb->len-IP_HDR_LEN, 0);
            skb_checksum(skb, IP_HDR_LEN, skb->len-IP_HDR_LEN, 0);
            skb_checksum(skb, IP_HDR_LEN, skb->len-IP_HDR_LEN, 0);
            for (i=0; i<10; i++) {
                memcpy(buf1, buf2, 1024);
            }

            test_count++;
            if ((test_count%10000) == 0) {
                printk("test_count=%d 0x%x\n", test_count, ntohl(daddr));
            }
        }
        return NF_ACCEPT;
    }

    return NF_ACCEPT;
}
#endif

static const struct nf_hook_ops nf_gtp_fw_ops[] = {
#ifdef GTP_TEST
    {
        .hook		= gtp_fw_process_test,
        .pf		    = NFPROTO_IPV4,
        .hooknum	= NF_INET_PRE_ROUTING,
        .priority	= NF_IP_PRI_FIRST,
    },
#else
    {
        .hook		= gtp_fw_process_pre_uplink,
        .pf		    = NFPROTO_IPV4,
        .hooknum	= NF_INET_PRE_ROUTING,
        //.hooknum	= NF_INET_LOCAL_IN,
        //.priority	= NF_IP_PRI_FIRST,
        .priority	= NF_IP_PRI_CONNTRACK_DEFRAG + 1,
    },
    {
        .hook		= gtp_fw_process_pre_downlink,
        .pf		    = NFPROTO_IPV4,
        .hooknum	= NF_INET_PRE_ROUTING,
        //.hooknum	= NF_INET_POST_ROUTING,
        .priority	= NF_IP_PRI_FILTER,
    },
#endif
};

static int nf_gtp_fw_init(void)
{
    int err = 0;

    if (gtp_fw__netlink_init() < 0) {
        printk("gtp_fw__netlink_init() failed!");
        return 0;
    }

    if (gtp_fl_ctx_init() != 0) {
        printk("gtp_fl_ctx_init() failed!");
        return 0;
    }

#ifdef FORCE_GW_DEV_CHECK
    g_gtp_dev = dev_get_by_name(&init_net, "pgwtun");
    if (g_gtp_dev == NULL) {
        printk("get pgwtun desc failed!");
        return 0;
    }
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    err = nf_register_net_hooks(&init_net, nf_gtp_fw_ops, ARRAY_SIZE(nf_gtp_fw_ops));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
    err = nf_register_hooks(nf_gtp_fw_ops, ARRAY_SIZE(nf_gtp_fw_ops));
#else
#endif

    if (err)
        return err;

    printk("nf_gtp_fw init success.");

    return 0;
}

static void nf_gtp_fw_exit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
    nf_unregister_net_hooks(&init_net, nf_gtp_fw_ops, ARRAY_SIZE(nf_gtp_fw_ops));
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
    nf_unregister_hooks(nf_gtp_fw_ops, ARRAY_SIZE(nf_gtp_fw_ops));
#else
#endif

    gtp_fw_netlink_exit();

    gtp_hashtable_free(&g_ctx);
}

module_init(nf_gtp_fw_init);
module_exit(nf_gtp_fw_exit);
MODULE_LICENSE("GPL");
