#define TRACE_MODULE _upf_gtp
#include "core_debug.h"
#include "gtp/gtp_types.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <libgtpnl/gtp.h>
#include <libgtpnl/gtpnl.h>
#include <libmnl/libmnl.h>
#include <errno.h>

static struct {
  int                 genl_id;
  struct mnl_socket  *nl;
  bool                is_enabled;
} gtp_nl;


#define GTP_DEVNAME "gtp0"

static status_t upf_libgtpnl_setup_intf(struct in_addr *ue_net, c_uint32_t ue_netmask, int mtu)
{
    struct in_addr ue_gw;
    char system_cmd[512] = {0};
    int fd0=-1, fd1u=-1;
    int ret=0;
    struct sockaddr_in sockaddr_fd0 = {
        .sin_family = AF_INET,
        .sin_port = htons(3386),
        .sin_addr = {
            .s_addr   = INADDR_ANY,
        },
    };
    struct sockaddr_in sockaddr_fd1 = {
        .sin_family = AF_INET,
        .sin_port = htons(GTPV1_U_UDP_PORT),
        .sin_addr = {
            .s_addr   = INADDR_ANY,
        },
    };

    // we don't need GTP v0, but interface with kernel requires 2 file descriptors
    fd0 = socket(AF_INET, SOCK_DGRAM, 0);
    fd1u = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(fd0, (struct sockaddr *) &sockaddr_fd0, sizeof(sockaddr_fd0)) < 0) {
        d_error("bind GTPv0 port failed!");
        goto error;
    }
    if (bind(fd1u, (struct sockaddr *) &sockaddr_fd1, sizeof(sockaddr_fd1)) < 0) {
        d_error("bind S1U port failed!");
        goto error;
    }

    if (gtp_dev_create(-1, GTP_DEVNAME, fd0, fd1u) < 0) {
        d_error("Cannot create GTP tunnel device: %s\n", strerror(errno));
        goto error;
    }
    gtp_nl.is_enabled = true;

    if ((gtp_nl.nl=genl_socket_open()) == NULL) {
        d_error("Cannot create genetlink socket\n");
        goto error;
    }

    if ((gtp_nl.genl_id = genl_lookup_family(gtp_nl.nl, "gtp")) < 0) {
        d_error("Cannot lookup GTP genetlink ID\n");
        goto error;
    }
    d_trace(5, "Using the GTP kernel mode (genl ID is %d)\n", gtp_nl.genl_id);

    sprintf(system_cmd, "ip link set dev %s mtu %u", GTP_DEVNAME, mtu);
    if ((ret=system (system_cmd)) != 0) {
        d_error("ERROR in system command %s: %d\n", system_cmd, ret);
        goto error;
    }

    ue_gw.s_addr = ue_net->s_addr | htonl(1);
    sprintf(system_cmd, "ip addr add %s/%u dev %s", inet_ntoa(ue_gw), ue_netmask, GTP_DEVNAME);
    if ((ret=system (system_cmd)) != 0) {
        d_error("ERROR in system command %s: %d\n", system_cmd, ret);
        goto error;
    }
    d_trace(5, "Setting route to reach UE net %s via %s\n", inet_ntoa(*ue_net), GTP_DEVNAME);

    if (gtp_dev_config(GTP_DEVNAME, ue_net, ue_netmask) < 0) {
        d_error("Cannot add route to reach network\n");
        goto error;
    }

    d_trace(5, "GTP kernel configured (marking required for dedicated bearers)\n");

    return CORE_OK;


error:
    close(fd0);
    close(fd1u);
    return CORE_ERROR;
}

status_t upf_libgtpnl_uninit(void)
{
    if (!gtp_nl.is_enabled)
        return -1;

    return gtp_dev_destroy(GTP_DEVNAME);
}

static status_t upf_libgtpnl_reset(void)
{
    status_t rv=CORE_OK;

    //rv = system ("modprobe --remove gtp");
    //rv = system ("modprobe gtp");
    return rv;
}

status_t upf_libgtpnl_add_tunnel(struct in_addr *ue,
                                 struct in_addr *enb, 
                                 c_uint32_t i_tei, 
                                 c_uint32_t o_tei, 
                                 c_uint8_t bearer_id)
{
    struct gtp_tunnel *t;
    int ret=CORE_OK;

    if (!gtp_nl.is_enabled)
        return CORE_ERROR;

    t = gtp_tunnel_alloc();
    if (t == NULL)
        return CORE_ERROR;


    gtp_tunnel_set_ifidx(t, if_nametoindex(GTP_DEVNAME));
    gtp_tunnel_set_version(t, 1);
    gtp_tunnel_set_ms_ip4(t, ue);
    gtp_tunnel_set_sgsn_ip4(t, enb);
    gtp_tunnel_set_i_tei(t, i_tei);
    gtp_tunnel_set_o_tei(t, o_tei);
    gtp_tunnel_set_flowid(t, bearer_id);

    ret = gtp_add_tunnel(gtp_nl.genl_id, gtp_nl.nl, t);
    gtp_tunnel_free(t);

    return ret;
}

status_t upf_libgtpnl_del_tunnel(struct in_addr *ue, c_uint32_t i_tei, c_uint32_t o_tei)
{
    struct gtp_tunnel *t;
    int ret=CORE_OK;

    if (!gtp_nl.is_enabled)
        return ret;

    t = gtp_tunnel_alloc();
    if (t == NULL)
        return CORE_ERROR;

    gtp_tunnel_set_ifidx(t, if_nametoindex(GTP_DEVNAME));
    gtp_tunnel_set_version(t, 1);
    // looking at kernel/drivers/net/gtp.c: not needed gtp_tunnel_set_ms_ip4(t, ue);
    // looking at kernel/drivers/net/gtp.c: not needed gtp_tunnel_set_sgsn_ip4(t, enb);
    gtp_tunnel_set_i_tei(t, i_tei);
    gtp_tunnel_set_o_tei(t, o_tei);

    ret = gtp_del_tunnel(gtp_nl.genl_id, gtp_nl.nl, t);
    gtp_tunnel_free(t);

    return ret;
}

status_t upf_libgtpnl_init()
{
    struct in_addr ue_net;
    c_uint32_t ue_netmask = 16;
    c_uint32_t mtu = 1500;

    inet_aton("45.45.0.0", &ue_net);
    if (upf_libgtpnl_reset() != CORE_OK) {
        d_error("insmod gtp.ko failed!");
        return CORE_ERROR;
    }
    
    if (upf_libgtpnl_setup_intf(&ue_net, ue_netmask, mtu) != CORE_OK) {
        d_error("upf_libgtpnl_setup_intf failed!");
        return CORE_ERROR;
    }

    return CORE_OK;
}
