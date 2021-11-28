#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include "gtp_fw.h"

#define NETLINK_GTP_FW    30

struct user_gtp_req {
    struct nlmsghdr hdr;
    struct gtp_flow flow;
};

unsigned int g_msg_seq = 0;

int cfg_to_kernel(int fd, struct sockaddr *daddr, GtpMsgType msg_type, struct gtp_flow *flow)
{
    int ret = 0;
    struct user_gtp_req req_msg;
    struct nlmsghdr resp_msg;
    struct nlmsghdr *nlh = (struct nlmsghdr *)&req_msg;
    int len=0;

    //printf("flow: %x-%d-%d", flow->inner_ip, flow->local_teid, flow->remote_teid);
    memset(&req_msg, 0, sizeof(struct user_gtp_req));

    nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct gtp_flow));
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = msg_type;
    nlh->nlmsg_seq = g_msg_seq;
    nlh->nlmsg_pid = getpid();

    memcpy(NLMSG_DATA(nlh), flow, sizeof(struct gtp_flow));
    ret = sendto(fd, nlh, nlh->nlmsg_len, 0, daddr, sizeof(struct sockaddr_nl));
    if (!ret) {
        printf("sendto error\n");
        g_msg_seq++;
        return -1;
    }

    memset(&resp_msg, 0, sizeof(struct nlmsghdr));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(fd, &resp_msg, sizeof(struct nlmsghdr), 0, daddr, &len);
    if (!ret) {
        printf("recv form kernel error\n");
        ret = -1;
    }

    if (resp_msg.nlmsg_type == GTP_MSG_RESP_FAILED) {
        printf("got GTP_MSG_RESP_FAILED form kernel\n");
        ret = -1;
    }

    if (resp_msg.nlmsg_seq  != g_msg_seq) {
        printf("msg_req is %u, expect %u\n", resp_msg.nlmsg_seq, g_msg_seq);
        ret = -1;
    }
    g_msg_seq++;

    return ret;
}

int main(int argc, char **argv)
{
    int skfd;
    int ret;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl saddr, daddr;
    struct gtp_flow flow;
    int i=0;
    struct in_addr ue_addr, enb_addr, epc_addr;

    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GTP_FW);
    if(skfd == -1)
    {
        perror("create socket error\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = getpid();
    saddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        perror("bind() error\n");
        close(skfd);
        return -1;
    }

    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel 
    daddr.nl_groups = 0;

    /*
    for(i=0; i<200; i++) {
        flow.remote_ip = 0x1e0c1412;
        flow.remote_port = 2152;
        flow.remote_teid = 0x10001+i;
        flow.inner_ip = 0x2d2d0002+i;
        flow.local_ip = 0x1e0c1410;
        flow.local_port = 2152;
        flow.local_teid = 1+i;
        cfg_to_kernel(skfd, (struct sockaddr *)&daddr, GTP_MSG_ADD_FLOW, &flow);
    }*/

    //ue
    inet_aton("45.45.0.2", &ue_addr);
    flow.inner_ip = ntohl(ue_addr.s_addr);

    // enb
    inet_aton("30.12.20.16", &enb_addr);
    flow.remote_ip = ntohl(enb_addr.s_addr);
    flow.remote_port = 2152;
    flow.remote_teid = 0x10001;

    // epc
    inet_aton("30.12.22.31", &epc_addr);
    flow.local_ip = ntohl(epc_addr.s_addr);
    flow.local_port = 2152;
    flow.local_teid = 1;

    cfg_to_kernel(skfd, (struct sockaddr *)&daddr, GTP_MSG_ADD_FLOW, &flow);
    cfg_to_kernel(skfd, (struct sockaddr *)&daddr, GTP_MSG_DUMP_FLOW, &flow);
    close(skfd);
    return 0;
}
