#define TRACE_MODULE _spgw_gtp_path
#include "core_debug.h"
#include "core_pkbuf.h"

#include "3gpp_types.h"
#include "gtp/gtp_node.h"
#include "gtp/gtp_path.h"
#include "gtp/gtp_conv.h"

#include "common/context.h"
#include "spgw_context.h"
#include "spgw_event.h"
#include "spgw_gtp_path.h"
#include "spgw_ipfw.h"
#include "core_epc_msg.h"

#define PGW_GTP_HANDLED     1
//#define GTP_TEST 1

c_uint16_t in_cksum(c_uint16_t *addr, int len);
#ifndef GTP_TEST
static status_t pgw_gtp_handle_multicast(pkbuf_t *recvbuf);
#endif
static status_t pgw_gtp_handle_slaac(spgw_sess_t *sess, pkbuf_t *recvbuf);
static status_t pgw_gtp_send_to_bearer(spgw_bearer_t *bearer, pkbuf_t *sendbuf);
static status_t pgw_gtp_send_router_advertisement(spgw_sess_t *sess, c_uint8_t *ip6_dst);


#ifdef GTP_TEST
#define ENB_S1U_TEID_TEST (0x1000001)
char *enb_ip_test = "30.12.22.29";

//#define ENB_S1U_TEID_TEST (0x1000002)
//char *enb_ip_test = "30.12.22.33";

static gtp_node_t *gnode_test=NULL;
static pgw_dev_t *dev_test=NULL;

static status_t pgw_gtp_send_test(pkbuf_t *sendbuf)
{
    status_t rv;
    gtp_header_t *gtp_h = NULL;
    gtp_f_teid_t enb_s1u_teid;
    ip_t enb_s1u_ip;

    /* Add GTP-U header */
    rv = pkbuf_header(sendbuf, GTPV1U_HEADER_LEN);
    if (rv != CORE_OK)
    {
        d_error("pkbuf_header error");
        pkbuf_free(sendbuf);
        return CORE_ERROR;
    }
    
    gtp_h = (gtp_header_t *)sendbuf->payload;
    gtp_h->flags = 0x30;
    gtp_h->type = GTPU_MSGTYPE_GPDU;
    gtp_h->length = htons(sendbuf->len - GTPV1U_HEADER_LEN);
    gtp_h->teid = htonl(ENB_S1U_TEID_TEST);

    /* Send to ENB */
    d_trace(50, "[SGW] SEND TO ENB: ");
    d_trace_hex(50, sendbuf->payload, sendbuf->len);

    if (gnode_test == NULL) {
        int len;

        memset(&enb_s1u_ip, 0, sizeof(ip_t));
        memset(&enb_s1u_teid, 0, sizeof(gtp_f_teid_t));
        enb_s1u_teid.interface_type = GTP_F_TEID_S1_U_ENODEB_GTP_U;
        enb_s1u_teid.teid = htonl(ENB_S1U_TEID_TEST);

        enb_s1u_ip.ipv4 = 1;
        inet_aton(enb_ip_test, (struct in_addr *)&enb_s1u_ip.addr);
        enb_s1u_ip.len =  4;

        rv = gtp_ip_to_f_teid(&enb_s1u_ip, &enb_s1u_teid, &len);
        d_assert(rv == CORE_OK, return CORE_ERROR,);

        gnode_test = gtp_add_node(&spgw_self()->enb_s1u_list, &enb_s1u_teid,
            spgw_self()->gtpu_port,
            context_self()->parameter.no_ipv4,
            context_self()->parameter.no_ipv6,
            context_self()->parameter.prefer_ipv4);
        d_assert(gnode_test, return CORE_ERROR,);

        rv = gtp_client(gnode_test);
        d_assert(rv == CORE_OK, return CORE_ERROR,);
    }
    rv =  gtp_send(gnode_test, sendbuf);

    return rv;
}
#endif

static int _gtpv1_tun_recv_cb(sock_id sock, void *data)
{
    pkbuf_t *recvbuf = NULL;
    int n;
#ifndef GTP_TEST
    status_t rv;
    spgw_bearer_t *bearer = NULL;
#endif

    recvbuf = pkbuf_alloc(GTPV1U_HEADER_LEN, MAX_SDU_LEN);
    d_assert(recvbuf, return -1, "pkbuf_alloc error");

    n = sock_read(sock, recvbuf->payload, recvbuf->len);
    if (n <= 0)
    {
        pkbuf_free(recvbuf);
        return -1;
    }

    recvbuf->len = n;

    d_trace(50, "[TUN] RECV : ");
    d_trace_hex(50, recvbuf->payload, recvbuf->len);

#ifdef GTP_TEST
    pgw_gtp_send_test(recvbuf);
#else
    /* Find the bearer by packet filter */
    bearer = pgw_bearer_find_by_packet(recvbuf);
    if (bearer)
    {
        /* Unicast */
        rv = pgw_gtp_send_to_bearer(bearer, recvbuf);
        d_assert(rv == CORE_OK,, "pgw_gtp_send_to_bearer() failed");
    }
    else
    {
        if (context_self()->parameter.multicast)
        {
            rv = pgw_gtp_handle_multicast(recvbuf);
            d_assert(rv != CORE_ERROR,, "pgw_gtp_handle_multicast() failed");
        }
    }
#endif

    pkbuf_free(recvbuf);
    return 0;

}

static int _gtpv2_c_recv_cb(sock_id sock, void *data)
{
    event_t e;
    status_t rv;
    pkbuf_t *pkbuf = NULL;

    d_assert(sock, return -1, "Null param");

    rv = gtp_recv(sock, &pkbuf);
    if (rv != CORE_OK)
    {
        if (errno == EAGAIN)
            return 0;

        return -1;
    }

    event_set(&e, SPGW_EVT_S11_MESSAGE);
    event_set_param1(&e, (c_uintptr_t)pkbuf);
    rv = spgw_event_send(&e);
    if (rv != CORE_OK)
    {
        d_error("pgw_event_send error");
        pkbuf_free(pkbuf);
        return 0;
    }

    return 0;
}

static int _gtpv1_u_recv_cb(sock_id sock, void *data)
{
    char buf[CORE_ADDRSTRLEN];
    status_t rv;
    pkbuf_t *pkbuf = NULL;
    c_sockaddr_t from;
    gtp_header_t *gtp_h = NULL;
    spgw_bearer_t *bearer = NULL;
    sgw_tunnel_t *tunnel = NULL;
    c_uint32_t teid;

    d_assert(sock, return -1, "Null param");

    rv = gtp_recvfrom(sock, &pkbuf, &from);
    if (rv != CORE_OK)
    {
        if (errno == EAGAIN)
            return 0;

        return -1;
    }

    d_trace(50, "[SPGW] RECV : ");
    d_trace_hex(50, pkbuf->payload, pkbuf->len);

    gtp_h = (gtp_header_t *)pkbuf->payload;
    if (gtp_h->type == GTPU_MSGTYPE_ECHO_REQ)
    {
        pkbuf_t *echo_rsp;

        d_trace(3, "[SPGW] RECV Echo Request from [%s]\n", CORE_ADDR(&from, buf));
        echo_rsp = gtp_handle_echo_req(pkbuf);
        if (echo_rsp)
        {
            ssize_t sent;

            /* Echo reply */
            d_trace(3, "[SPGW] SEND Echo Response to [%s]\n", CORE_ADDR(&from, buf));

            sent = core_sendto(sock, echo_rsp->payload, echo_rsp->len, 0, &from);
            if (sent < 0 || sent != echo_rsp->len)
            {
                d_error("core_sendto failed(%d:%s)", errno, strerror(errno));
            }
            pkbuf_free(echo_rsp);
        }
    }
    else if (gtp_h->type == GTPU_MSGTYPE_GPDU || gtp_h->type == GTPU_MSGTYPE_END_MARKER)
    {
#ifdef GTP_TEST
        {
            c_uint32_t size = GTPV1U_HEADER_LEN;

            if (gtp_h->flags & GTPU_FLAGS_S) size += 4;
            d_assert(pkbuf_header(pkbuf, -size) == CORE_OK, goto cleanup,);

            if (dev_test == NULL) {
                dev_test = pgw_dev_find_by_ifname("pgwtun");
            }
            d_assert(dev_test, goto cleanup,);

            if (sock_write(dev_test->sock, pkbuf->payload, pkbuf->len) <= 0)
                d_error("sock_write() failed");
            goto cleanup;
        }
#endif
        teid = ntohl(gtp_h->teid);
        if (gtp_h->type == GTPU_MSGTYPE_GPDU)
            d_trace(3, "[SPGW] RECV GPU-U from [%s] : TEID[0x%x]\n", CORE_ADDR(&from, buf), teid);
        else if (gtp_h->type == GTPU_MSGTYPE_END_MARKER)
            d_trace(3, "[SPGW] RECV End Marker from [%s] : TEID[0x%x]\n", CORE_ADDR(&from, buf), teid);

        tunnel = sgw_tunnel_find_by_teid(teid);
        if (!tunnel)
        {
            if (gtp_h->type == GTPU_MSGTYPE_GPDU)
                d_warn("[SPGW] RECV GPU-U from [%s] : No TEID[0x%x]", CORE_ADDR(&from, buf), teid);
            else if (gtp_h->type == GTPU_MSGTYPE_END_MARKER)
                d_warn("[SPGW] RECV End Marker from [%s] : No TEID[0x%x]", CORE_ADDR(&from, buf), teid);
            pkbuf_free(pkbuf);
            return 0;
        }
        bearer = tunnel->bearer;
        d_assert(bearer, pkbuf_free(pkbuf); return 0, "Null param");

        if (tunnel->interface_type == GTP_F_TEID_S1_U_SGW_GTP_U)
        {
            struct ip *ip_h = NULL;
            c_uint32_t size = GTPV1U_HEADER_LEN;
            spgw_sess_t *sess = NULL;
            pgw_subnet_t *subnet = NULL;
            pgw_dev_t *dev = NULL;
            c_uint32_t is_ipv4 = 0;

            if (gtp_h->flags & GTPU_FLAGS_S) size += 4;
            /* Remove GTP header and send packets to TUN interface */
            d_assert(pkbuf_header(pkbuf, -size) == CORE_OK, goto cleanup,);

            ip_h = pkbuf->payload;
            d_assert(ip_h, goto cleanup,);

            sess = bearer->sess;
            d_assert(sess, goto cleanup,);

            if (ip_h->ip_v == 4 && sess->ipv4) {
                subnet = sess->ipv4->subnet;
                is_ipv4 = 1;
            } else if (ip_h->ip_v == 6 && sess->ipv6) {
                subnet = sess->ipv6->subnet;
            }

            if (!subnet) {
                d_trace_hex(9, pkbuf->payload, pkbuf->len);
                d_trace(9, "[DROP] Cannot find subnet V:%d, IPv4:%p, IPv6:%p\n", ip_h->ip_v, sess->ipv4, sess->ipv6);
                goto cleanup;
            }

            /* Check IPv6 */
            if (context_self()->parameter.no_slaac == 0 && ip_h->ip_v == 6) {
                rv = pgw_gtp_handle_slaac(sess, pkbuf);
                if (rv == PGW_GTP_HANDLED) {
                    pkbuf_free(pkbuf);
                    return 0;
                }
                d_assert(rv == CORE_OK,, "pgw_gtp_handle_slaac() failed");
            }

            dev = subnet->dev;
            d_assert(dev, goto cleanup,);
            
            /* uplink */
            if (is_ipv4 == 1) {
                spgw_ue_flow_stat_update(sess->ipv4->addr[0], pkbuf->len, 0);
            }
            
            if (sock_write(dev->sock, pkbuf->payload, pkbuf->len) <= 0)
                d_error("sock_write() failed");
        }
        else if (tunnel->interface_type == GTP_F_TEID_SGW_GTP_U_FOR_DL_DATA_FORWARDING ||
                tunnel->interface_type == GTP_F_TEID_SGW_GTP_U_FOR_UL_DATA_FORWARDING)
        {
            sgw_tunnel_t *indirect_tunnel = NULL;

            indirect_tunnel = sgw_tunnel_find_by_interface_type(bearer, tunnel->interface_type);
            d_assert(indirect_tunnel, pkbuf_free(pkbuf); return 0,);
            d_assert(indirect_tunnel->gnode, pkbuf_free(pkbuf); return 0,);
            d_assert(indirect_tunnel->gnode->sock, pkbuf_free(pkbuf); return 0,);
            d_trace(3, "[SPGW] SEND GPU-U to Indirect Tunnel[%s]: TEID[0x%x]\n",
                CORE_ADDR(sock_remote_addr(indirect_tunnel->gnode->sock), buf),
                indirect_tunnel->remote_teid);

            gtp_h->teid = htonl(indirect_tunnel->remote_teid);
            gtp_send(indirect_tunnel->gnode, pkbuf);
        }
        else if (tunnel->interface_type == GTP_F_TEID_S5_S8_SGW_GTP_U)
        {
            d_error("receive gtp-u from s5u/s8u, something is wrong!");
        }
    }

cleanup:
    pkbuf_free(pkbuf);
    return 0;
}

status_t pgw_gtp_open()
{
    status_t rv;
    pgw_dev_t *dev = NULL;
    pgw_subnet_t *subnet = NULL;
    int rc;

    rv = gtp_server_list(&spgw_self()->gtpc_list, _gtpv2_c_recv_cb);
    d_assert(rv == CORE_OK, return CORE_ERROR,);
    rv = gtp_server_list(&spgw_self()->gtpc_list6, _gtpv2_c_recv_cb);
    d_assert(rv == CORE_OK, return CORE_ERROR,);

    spgw_self()->gtpc_sock = gtp_local_sock_first(&spgw_self()->gtpc_list);
    spgw_self()->gtpc_sock6 = gtp_local_sock_first(&spgw_self()->gtpc_list6);
    spgw_self()->gtpc_addr = gtp_local_addr_first(&spgw_self()->gtpc_list);
    spgw_self()->gtpc_addr6 = gtp_local_addr_first(&spgw_self()->gtpc_list6);

    d_assert(spgw_self()->gtpc_addr || spgw_self()->gtpc_addr6,
            return CORE_ERROR, "No GTP Server");

    rv = gtp_server_list(&spgw_self()->gtpu_list, _gtpv1_u_recv_cb);
    d_assert(rv == CORE_OK, return CORE_ERROR,);
    rv = gtp_server_list(&spgw_self()->gtpu_list6, _gtpv1_u_recv_cb);
    d_assert(rv == CORE_OK, return CORE_ERROR,);

    spgw_self()->gtpu_sock = gtp_local_sock_first(&spgw_self()->gtpu_list);
    spgw_self()->gtpu_sock6 = gtp_local_sock_first(&spgw_self()->gtpu_list6);
    spgw_self()->gtpu_addr = gtp_local_addr_first(&spgw_self()->gtpu_list);
    spgw_self()->gtpu_addr6 = gtp_local_addr_first(&spgw_self()->gtpu_list6);

    d_assert(spgw_self()->gtpu_addr || spgw_self()->gtpu_addr6,
            return CORE_ERROR, "No GTP Server");


    /* NOTE : tun device can be created via following command.
     *
     * $ sudo ip tuntap add name pgwtun mode tun
     *
     * Also, before running pgw, assign the one IP from IP pool of UE 
     * to pgwtun. The IP should not be assigned to UE
     *
     * $ sudo ifconfig pgwtun 45.45.0.1/16 up
     *
     */

    /* Open Tun interface */
    for (dev = pgw_dev_first(); dev; dev = pgw_dev_next(dev))
    {
        rc = tun_open(&dev->sock, (char *)dev->ifname, 0);
        if (rc != 0)
        {
            d_error("tun_open(dev:%s) failed", dev->ifname);
            return CORE_ERROR;
        }

        rc = sock_register(dev->sock, _gtpv1_tun_recv_cb, NULL);
        if (rc != 0)
        {
            d_error("sock_register(dev:%s) failed", dev->ifname);
            sock_delete(dev->sock);
            return CORE_ERROR;
        }
    }

    /* 
     * On Linux, it is possible to create a persistent tun/tap 
     * interface which will continue to exist even if vepc quit, 
     * although this is normally not required. 
     * It can be useful to set up a tun/tap interface owned 
     * by a non-root user, so vepc can be started without 
     * needing any root privileges at all.
     */

    /* Set P-to-P IP address with Netmask
     * Note that Linux will skip this configuration */
    for (subnet = pgw_subnet_first(); subnet; subnet = pgw_subnet_next(subnet))
    {
        d_assert(subnet->dev, return CORE_ERROR,);
        rc = tun_set_ip(subnet->dev->sock, &subnet->gw, &subnet->sub);
        if (rc != 0)
        {
            d_error("tun_set_ip(dev:%s) failed", subnet->dev->ifname);
            return CORE_ERROR;
        }
    }

    /* Link-Local Address for PGW_TUN */
    for (dev = pgw_dev_first(); dev; dev = pgw_dev_next(dev))
        dev->link_local_addr = core_link_local_addr_by_dev(dev->ifname);

    return CORE_OK;
}

status_t pgw_gtp_close()
{
    pgw_dev_t *dev = NULL;

    sock_delete_list(&spgw_self()->gtpc_list);
    sock_delete_list(&spgw_self()->gtpc_list6);

    sock_delete_list(&spgw_self()->gtpu_list);
    sock_delete_list(&spgw_self()->gtpu_list6);

    for (dev = pgw_dev_first(); dev; dev = pgw_dev_next(dev))
        sock_delete(dev->sock);

    return CORE_OK;
}

#ifndef GTP_TEST
static status_t pgw_gtp_handle_multicast(pkbuf_t *recvbuf)
{
    status_t rv;
    struct ip *ip_h =  NULL;
    struct ip6_hdr *ip6_h =  NULL;

    ip_h = (struct ip *)recvbuf->payload;
    if (ip_h->ip_v == 6)
    {
#if COMPILE_ERROR_IN_MAC_OS_X  /* Compiler error in Mac OS X platform */
        ip6_h = (struct ip6_hdr *)recvbuf->payload;
        if (IN6_IS_ADDR_MULTICAST(&ip6_h->ip6_dst))
#else
        struct in6_addr ip6_dst;
        ip6_h = (struct ip6_hdr *)recvbuf->payload;
        memcpy(&ip6_dst, &ip6_h->ip6_dst, sizeof(struct in6_addr));
        if (IN6_IS_ADDR_MULTICAST(&ip6_dst))
#endif
        {
            hash_index_t *hi = NULL;

            /* IPv6 Multicast */
            for (hi = pgw_sess_first(); hi; hi = pgw_sess_next(hi))
            {
                spgw_sess_t *sess = spgw_sess_this(hi);
                d_assert(sess, return CORE_ERROR,);
                if (sess->ipv6)
                {
                    /* PDN IPv6 is avaiable */
                    spgw_bearer_t *bearer = pgw_default_bearer_in_sess(sess);
                    d_assert(bearer, return CORE_ERROR,);

                    rv = pgw_gtp_send_to_bearer(bearer, recvbuf);
                    d_assert(rv == CORE_OK,,"pgw_gtp_send_to_bearer failed");

                    return PGW_GTP_HANDLED;
                }
            }
        }
    }

    return CORE_OK;
}
#endif

static status_t pgw_gtp_handle_slaac(spgw_sess_t *sess, pkbuf_t *recvbuf)
{
    status_t rv;
    struct ip *ip_h = NULL;

    d_assert(sess, return CORE_ERROR,);
    d_assert(recvbuf, return CORE_ERROR,);
    d_assert(recvbuf->payload, return CORE_ERROR,);
    ip_h = (struct ip *)recvbuf->payload;
    if (ip_h->ip_v == 6)
    {
        struct ip6_hdr *ip6_h = (struct ip6_hdr *)recvbuf->payload;
        if (ip6_h->ip6_nxt == IPPROTO_ICMPV6)
        {
            struct icmp6_hdr *icmp_h =
                (struct icmp6_hdr *)(recvbuf->payload + sizeof(struct ip6_hdr));
            if (icmp_h->icmp6_type == ND_ROUTER_SOLICIT)
            {
                d_trace(5, "[SPGW]      Router Solict\n");
                if (sess->ipv6)
                {
                    rv = pgw_gtp_send_router_advertisement(
                            sess, ip6_h->ip6_src.s6_addr);
                    d_assert(rv == CORE_OK,,"send router advertisement failed");
                }
                return PGW_GTP_HANDLED;
            }
        }
    }

    return CORE_OK;
}

static status_t pgw_gtp_send_to_bearer(spgw_bearer_t *bearer, pkbuf_t *sendbuf)
{
    //char buf[CORE_ADDRSTRLEN];
    status_t rv;
    gtp_header_t *gtp_h = NULL;
    sgw_tunnel_t *s1u_tunnel = NULL;

    d_assert(bearer, pkbuf_free(sendbuf); return CORE_ERROR,);
    //d_assert(bearer->gnode, pkbuf_free(sendbuf); return CORE_ERROR,);
    //d_assert(bearer->gnode->sock, pkbuf_free(sendbuf); return CORE_ERROR,);
    
    s1u_tunnel = sgw_s1u_tunnel_in_bearer(bearer);
    d_assert(s1u_tunnel, pkbuf_free(sendbuf); return CORE_ERROR,);

    /* Add GTP-U header */
    rv = pkbuf_header(sendbuf, GTPV1U_HEADER_LEN);
    if (rv != CORE_OK)
    {
        d_error("pkbuf_header error");
        pkbuf_free(sendbuf);
        return CORE_ERROR;
    }
    
    gtp_h = (gtp_header_t *)sendbuf->payload;
    /* Bits    8  7  6  5  4  3  2  1
     *        +--+--+--+--+--+--+--+--+
     *        |version |PT| 1| E| S|PN|
     *        +--+--+--+--+--+--+--+--+
     *         0  0  1   1  0  0  0  0
     */
    gtp_h->flags = 0x30;
    gtp_h->type = GTPU_MSGTYPE_GPDU;
    gtp_h->length = htons(sendbuf->len - GTPV1U_HEADER_LEN);
    gtp_h->teid = htonl(s1u_tunnel->remote_teid);

    /* Send to ENB */
    d_trace(50, "[SPGW] SEND TO ENB: ");
    d_trace_hex(50, sendbuf->payload, sendbuf->len);


    if (s1u_tunnel->remote_teid) {
        int i=0;

        d_assert(s1u_tunnel->gnode, pkbuf_free(sendbuf); return CORE_ERROR,);
        d_assert(s1u_tunnel->gnode->sock, pkbuf_free(sendbuf); return CORE_ERROR,);
        
        /* If there is buffered packet, send it first */
        for (i = 0; i < bearer->num_buffered_pkt; i++)
        {
            gtp_header_t *gtp_h_2 = NULL;

            gtp_h_2 = (gtp_header_t *)bearer->buffered_pkts[i]->payload;
            gtp_h_2->teid =  htonl(s1u_tunnel->remote_teid);

            gtp_send(s1u_tunnel->gnode, bearer->buffered_pkts[i]);
            pkbuf_free(bearer->buffered_pkts[i]);
        }
        bearer->num_buffered_pkt = 0;

        rv =  gtp_send(s1u_tunnel->gnode, sendbuf);
    } else {
        /*
         * S1U path is deactivated.
         * Send downlink_data_notification to MME.
         */
        sgw_ue_t *sgw_ue = NULL;

        d_assert(bearer->sess, pkbuf_free(sendbuf); return CORE_ERROR,);
        d_assert(bearer->sess->sgw_ue, pkbuf_free(sendbuf); return CORE_ERROR,);

        sgw_ue = bearer->sess->sgw_ue;

        d_trace(3, "[SPGW] S1U PATH deactivated : STATE[0x%x]\n", SGW_GET_UE_STATE(sgw_ue));
        if ((SGW_GET_UE_STATE(sgw_ue) & SGW_S1U_INACTIVE))
        {
            d_trace(5, "    SPGW-S1U Inactive\n");
            if (!(SGW_GET_UE_STATE(sgw_ue) & SGW_DL_NOTI_SENT))
            {
                event_t e;
                status_t rv;

                d_trace(5, "    EVENT DL Data Notification\n");
                event_set(&e, SPGW_EVT_LO_DLDATA_NOTI);
                event_set_param1(&e, (c_uintptr_t)bearer->index);
                rv = spgw_event_send(&e);
                if (rv != CORE_OK)
                {
                    d_error("spgw_event_send error");
                    return CORE_OK;
                }

                SGW_SET_UE_STATE(sgw_ue, SGW_DL_NOTI_SENT);
            }

            /* TODO: Buffer the packet */
            if (bearer->num_buffered_pkt < MAX_NUM_BUFFER_PKT)
            {
                bearer->buffered_pkts[bearer->num_buffered_pkt++] = sendbuf;
                return CORE_OK;
            }
        }
        else
        {
            /* UE is S1U_ACTIVE state but there is no s1u teid */
            d_trace(5, "[SPGW] UE is ACITVE but there is no matched ENB_S1U_TEID[%d]", s1u_tunnel->local_teid);
            /* Just drop it */
        }
    }

    return rv;
}

static status_t pgw_gtp_send_router_advertisement(
        spgw_sess_t *sess, c_uint8_t *ip6_dst)
{
    status_t rv;
    pkbuf_t *pkbuf = NULL;

    spgw_bearer_t *bearer = NULL;
    pgw_ue_ip_t *ue_ip = NULL;
    pgw_subnet_t *subnet = NULL;
    pgw_dev_t *dev = NULL;

    ipsubnet_t src_ipsub;
    c_uint16_t plen = 0;
    c_uint8_t nxt = 0;
    c_uint8_t *p = NULL;
    struct ip6_hdr *ip6_h =  NULL;
    struct nd_router_advert *advert_h = NULL;
    struct nd_opt_prefix_info *prefix = NULL;

    d_assert(sess, return CORE_ERROR,);
    bearer = pgw_default_bearer_in_sess(sess);
    d_assert(bearer, return CORE_ERROR,);
    ue_ip = sess->ipv6;
    d_assert(ue_ip, return CORE_ERROR,);
    subnet = ue_ip->subnet;
    d_assert(subnet, return CORE_ERROR,);
    dev = subnet->dev;
    d_assert(dev, return CORE_ERROR,);

    pkbuf = pkbuf_alloc(GTPV1U_HEADER_LEN, 200);
    d_assert(pkbuf, return CORE_ERROR,);
    pkbuf->len = sizeof *ip6_h + sizeof *advert_h + sizeof *prefix;
    memset(pkbuf->payload, 0, pkbuf->len);

    p = (c_uint8_t *)pkbuf->payload;
    ip6_h = (struct ip6_hdr *)p;
    advert_h = (struct nd_router_advert *)((c_uint8_t *)ip6_h + sizeof *ip6_h);
    prefix = (struct nd_opt_prefix_info *)
        ((c_uint8_t*)advert_h + sizeof *advert_h);

    rv = core_ipsubnet(&src_ipsub, "fe80::1", NULL);
    d_assert(rv == CORE_OK, return CORE_ERROR,);
    if (dev->link_local_addr)
        memcpy(src_ipsub.sub, dev->link_local_addr->sin6.sin6_addr.s6_addr,
                sizeof src_ipsub.sub);

    advert_h->nd_ra_type = ND_ROUTER_ADVERT;
    advert_h->nd_ra_code = 0;
    advert_h->nd_ra_curhoplimit = 64;
    advert_h->nd_ra_flags_reserved = 0;
    advert_h->nd_ra_router_lifetime = htons(64800);  /* 64800s */
    advert_h->nd_ra_reachable = 0;
    advert_h->nd_ra_retransmit = 0;

    prefix->nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
    prefix->nd_opt_pi_len = 4; /* 32bytes */
    prefix->nd_opt_pi_prefix_len = subnet->prefixlen;
    prefix->nd_opt_pi_flags_reserved =
        ND_OPT_PI_FLAG_ONLINK|ND_OPT_PI_FLAG_AUTO;
    prefix->nd_opt_pi_valid_time = htonl(0xffffffff); /* Infinite */
    prefix->nd_opt_pi_preferred_time = htonl(0xffffffff); /* Infinite */
    memcpy(prefix->nd_opt_pi_prefix.s6_addr,
            subnet->sub.sub, sizeof prefix->nd_opt_pi_prefix.s6_addr);

    /* For IPv6 Pseudo-Header */
    plen = htons(sizeof *advert_h + sizeof *prefix);
    nxt = IPPROTO_ICMPV6;

    memcpy(p, src_ipsub.sub, sizeof src_ipsub.sub);
    p += sizeof src_ipsub.sub;
    memcpy(p, ip6_dst, IPV6_LEN);
    p += IPV6_LEN;
    p += 2; memcpy(p, &plen, 2); p += 2;
    p += 3; *p = nxt; p += 1;
    advert_h->nd_ra_cksum = in_cksum((c_uint16_t *)pkbuf->payload, pkbuf->len);

    ip6_h->ip6_flow = htonl(0x60000001);
    ip6_h->ip6_plen = plen;
    ip6_h->ip6_nxt = nxt;  /* ICMPv6 */
    ip6_h->ip6_hlim = 0xff;
    memcpy(ip6_h->ip6_src.s6_addr, src_ipsub.sub, sizeof src_ipsub.sub);
    memcpy(ip6_h->ip6_dst.s6_addr, ip6_dst, IPV6_LEN);
    
    rv = pgw_gtp_send_to_bearer(bearer, pkbuf);
    d_assert(rv == CORE_OK,, "pgw_gtp_send_to_bearer() faild");

    d_trace(5, "[SPGW]      Router Advertisement\n");

    pkbuf_free(pkbuf);
    return rv;
}

c_uint16_t in_cksum(c_uint16_t *addr, int len)
{
    int nleft = len;
    c_uint32_t sum = 0;
    c_uint16_t *w = addr;
    c_uint16_t answer = 0;

    // Adding 16 bits sequentially in sum
    while (nleft > 1)
    {
        sum += *w;
        nleft -= 2;
        w++;
    }

    // If an odd byte is left
    if (nleft == 1)
    {
        *(c_uint8_t *) (&answer) = *(c_uint8_t *) w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

status_t sgw_gtp_send_end_marker(sgw_tunnel_t *s1u_tunnel)
{
    char buf[CORE_ADDRSTRLEN];
    status_t rv;
    pkbuf_t *pkbuf = NULL;
    gtp_header_t *h = NULL;

    d_assert(s1u_tunnel, return CORE_ERROR,);
    d_assert(s1u_tunnel->gnode, return CORE_ERROR,);
    d_assert(s1u_tunnel->gnode->sock, return CORE_ERROR,);

    d_trace(3, "[SPGW] SEND End Marker to ENB[%s]: TEID[0x%x]\n",
        CORE_ADDR(sock_remote_addr(s1u_tunnel->gnode->sock), buf),
        s1u_tunnel->remote_teid);

    pkbuf = pkbuf_alloc(0, 100 /* enough for END_MARKER; use smaller buffer */);
    d_assert(pkbuf, return CORE_ERROR,);
    h = (gtp_header_t *)pkbuf->payload;

    memset(h, 0, GTPV1U_HEADER_LEN);

    /*
     * Flags
     * 0x20 - Version : GTP release 99 version (1)
     * 0x10 - Protocol Type : GTP (1)
     */
    h->flags = 0x30;
    h->type = GTPU_MSGTYPE_END_MARKER;
    h->teid =  htonl(s1u_tunnel->remote_teid);
    
    rv = gtp_send(s1u_tunnel->gnode, pkbuf);
    d_assert(rv == CORE_OK,, "gtp send failed");
    pkbuf_free(pkbuf);

    return rv;
}

status_t pgw_update_s5u_gtp(int action,
                            c_uint8_t *imsi,
                            int imsi_len,
                            c_uint8_t ebi,
                            c_uint32_t ue_ip, 
                            c_uint32_t sgw_s5u_teid, 
                            c_uint32_t pgw_s5u_teid)
{
    char buf[sizeof(EpcMsgHeader)+sizeof(struct gtp_u_ctx)] = {0};
    EpcMsgHeader *msg = (EpcMsgHeader *)buf;
    struct gtp_u_ctx *gtp_info = (struct gtp_u_ctx *)(msg+1);
    if (action == 1) {
        msg->type = MSG_T_GTP_S5U_ADD;
    } else {
        msg->type = MSG_T_GTP_S5U_DEL;
    }
    msg->dataLength = sizeof(struct gtp_u_ctx);

    memcpy(&gtp_info->imsi, imsi, MAX_IMSI_LEN);
    gtp_info->imsi_len = imsi_len;
    gtp_info->ebi = ebi;
    gtp_info->ue_ip = ue_ip;
    gtp_info->sgw_s5u_teid = sgw_s5u_teid;
    gtp_info->pgw_s5u_teid = pgw_s5u_teid;
    
    if (epc_msg_send_once(EID_UDF, msg) != 0) {
        d_trace(1, "pgw update s5u gtp(%d[%d]: %x %d-%d) failed!\n", ebi, action, ue_ip, sgw_s5u_teid, pgw_s5u_teid);
        return CORE_ERROR;
    }
    //d_trace(1, "pgw update s5u gtp(%d[%d]: %x %d-%d) success.\n", ebi, action, ue_ip, sgw_s5u_teid, pgw_s5u_teid);

    return CORE_OK;
}
