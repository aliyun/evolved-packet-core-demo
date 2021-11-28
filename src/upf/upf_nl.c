#define TRACE_MODULE _upf_nl
#include "core_debug.h"
#include "core_thread.h"
#include "upf_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>
//#include <arpa/inet.h>
#include "upf_nl.h"

#define NETLINK_GTP_FW    30

struct user_gtp_req {
    struct nlmsghdr hdr;
    struct gtp_flow flow;
};

static int g_fd = -1;
static unsigned int g_msg_seq = 0;

int cfg_to_kernel(int fd, GtpMsgType msg_type, struct gtp_flow *flow)
{
    int ret = 0;
    struct user_gtp_req req_msg;
    struct nlmsghdr resp_msg;
    struct nlmsghdr *nlh = (struct nlmsghdr *)&req_msg;
    socklen_t len;
    struct sockaddr_nl daddr;

    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel 
    daddr.nl_groups = 0;

    memset(&req_msg, 0, sizeof(struct user_gtp_req));

    nlh->nlmsg_len = NLMSG_SPACE(sizeof(struct gtp_flow));
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = msg_type;
    nlh->nlmsg_seq = g_msg_seq;
    nlh->nlmsg_pid = getpid();

    memcpy(NLMSG_DATA(nlh), flow, sizeof(struct gtp_flow));
    ret = sendto(fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if (!ret) {
        printf("sendto error\n");
        g_msg_seq++;
        return -1;
    }

    memset(&resp_msg, 0, sizeof(struct nlmsghdr));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(fd, &resp_msg, sizeof(struct nlmsghdr), 0, (struct sockaddr *)&daddr, &len);
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


int upf_nl_init()
{
    struct sockaddr_nl saddr;

    g_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GTP_FW);
    if(g_fd == -1) {
        d_trace(1, "create nl socket error!\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = getpid();
    saddr.nl_groups = 0;
    if(bind(g_fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        d_trace(1, "nl sock bind error!\n");
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    return 0;
}

int upf_add_flow(struct gtp_flow *flow)
{
    int ret=0;

    if (g_fd == -1) {
        if (upf_nl_init() < 0) {
            d_trace(1, "upf_nl_init failed!");
            return -1;
        }
    }

    if ((ret=cfg_to_kernel(g_fd, GTP_MSG_ADD_FLOW, flow)) < 0) {
        d_trace(1, "add upf flow failed! ret=%d", ret);
    }
    cfg_to_kernel(g_fd, GTP_MSG_DUMP_FLOW, flow);
    close(g_fd);
    g_fd = -1;
    
    return ret;
}
