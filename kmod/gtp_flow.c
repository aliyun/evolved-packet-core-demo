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

int send_msg_to_kernel(int fd, GtpMsgType msg_type, char *data, int len)
{
    struct sockaddr_nl saddr, daddr;
    struct user_gtp_req req_msg;
    struct nlmsghdr *nlh = (struct nlmsghdr *)&req_msg;
    int ret;

    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel 
    daddr.nl_groups = 0;

    nlh->nlmsg_len = NLMSG_SPACE(len);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = msg_type;
    nlh->nlmsg_seq = g_msg_seq;
    nlh->nlmsg_pid = getpid();

    if (len > 0) {
        memcpy(NLMSG_DATA(nlh), data, len);
    }

    ret = sendto(fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if (ret != nlh->nlmsg_len) {
        printf("sendto error, ret=%d\n", nlh->nlmsg_len);
        g_msg_seq++;
        return -1;
    }
    g_msg_seq++;

    return 0;
}

int init_nl_socket()
{
    struct sockaddr_nl saddr;
    int fd;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GTP_FW);
    if (fd < 0) {
        printf("create socket error\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = getpid();
    saddr.nl_groups = 0;
    if(bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        printf("bind() error\n");
        close(fd);
        return -1;
    }

    return fd;
}

int add_del_flow(int is_add, struct gtp_flow *flow)
{
    int fd;
    int len=0;
    int ret=0;
    struct sockaddr_nl saddr, daddr;
    struct nlmsghdr resp_msg;

    if ((fd=init_nl_socket()) < 0) {
        printf("init_nl_socket failed\n");
        return -1;
    }

    if (send_msg_to_kernel(fd, is_add?GTP_MSG_ADD_FLOW:GTP_MSG_DEL_FLOW, (char *)flow, sizeof(struct gtp_flow)) != 0) {
        printf("send_msg_to_kernel failed\n");
        close(fd);
        return -1;
    }


    memset(&daddr, 0, sizeof(daddr));
    memset(&resp_msg, 0, sizeof(struct nlmsghdr));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(fd, &resp_msg, sizeof(struct nlmsghdr), 0, (struct sockaddr *)&daddr, &len);
    if (ret != sizeof(struct nlmsghdr)) {
        printf("recv form kernel error. ret=%d\n", ret);
        goto failed;
    }

    if (resp_msg.nlmsg_type == GTP_MSG_RESP_FAILED) {
        printf("got GTP_MSG_RESP_FAILED form kernel\n");
        goto failed;
    }

    if (resp_msg.nlmsg_seq != (g_msg_seq-1)) {
        printf("msg_req is %u, expect %u\n", resp_msg.nlmsg_seq, g_msg_seq - 1);
        goto failed;
    }

    close(fd);
    return 0;

failed:
    close(fd);
    return -1;
}

int dump_flows()
{
    int fd;
    int keep_loop = 1;
    int len=0,i=0,ret=0;
    struct sockaddr_nl daddr;
    char buf[sizeof(struct nlmsghdr) + sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG] = {0};
    struct nlmsghdr *resp_msg=(struct nlmsghdr *)&buf;
    struct gtp_flow *flow = NULL;
    int recv_len = sizeof(struct nlmsghdr) + sizeof(struct gtp_flow)*MAX_FLOW_CNT_IN_MSG;

    if ((fd=init_nl_socket()) < 0) {
        printf("init_nl_socket failed\n");
        return -1;
    }

    if (send_msg_to_kernel(fd, GTP_MSG_GET_ALL_FLOW, (char *)&buf, sizeof(struct gtp_flow)) != 0) {
        printf("send_msg_to_kernel failed\n");
        close(fd);
        return -1;
    }

    memset(&daddr, 0, sizeof(daddr));
    memset(resp_msg, 0, recv_len);
    len = sizeof(struct sockaddr_nl);
    printf("\ninner_ip-remote[ip:port:teid]-local[ip:port:teid]\tencap_bytes\tdecap_bytes\tflags\n");
    while(keep_loop) {
        ret = recvfrom(fd, resp_msg, recv_len, 0, (struct sockaddr *)&daddr, &len);
        if (ret != recv_len) {
            printf("recv nl msg failed! ret=%d err=%s\n", ret, strerror(errno));
            break;
        }


        flow = (struct gtp_flow *)NLMSG_DATA(resp_msg);
        for (i=0; i<MAX_FLOW_CNT_IN_MSG; i++) {
            if (flow->inner_ip == 0) {
                keep_loop = 0;
                break;
            }

            printf("0x%x-0x%x:%d:%d-0x%x:%d:%d\t%d\t%d\t0x%x\n", flow->inner_ip,
                                                                 flow->remote_ip,
                                                                 flow->remote_port,
                                                                 flow->remote_teid,
                                                                 flow->local_ip,
                                                                 flow->local_port,
                                                                 flow->local_teid,
                                                                 flow->encap_bytes,
                                                                 flow->decap_bytes,
                                                                 flow->flags);
            flow = flow + 1;
        }
    }
    printf("\n");

    close(fd);
    return 0;
}

void show_usage()
{
    printf("Usage:\n");
    printf("\tgtp_flow add [inner_ip]-[remote_ip]:[remote_port]:[remote_teid]-[local_ip]:[local_port]:[local_teid]\n");
    printf("\tgtp_flow del [inner_ip]-[remote_ip]:[remote_port]:[remote_teid]-[local_ip]:[local_port]:[local_teid]\n");
    printf("\tgtp_flow show\n\n");
}

int parse_flow_from_arg(char *flow_string, struct gtp_flow *flow)
{
    char *p;
    char *inner_ip,*remote,*local;
    char *remote_ip,*remote_port,*remote_teid;
    char *local_ip,*local_port,*local_teid;
    struct in_addr addr;
    int port;
    int teid;

    if (!flow_string) {
        printf("flow info is needed!\n\n");
        return -1;
    }

    // param: inner_ip
    inner_ip = strtok(flow_string, "-");
    if (!inner_ip) {
        printf("inner_ip is needed!\n\n");
        return -1;
    }

    if (!inet_aton(inner_ip, &addr)) {
        printf("inner_ip is invalid!\n\n");
        return -1;
    }
    flow->inner_ip = ntohl(addr.s_addr);

    // remote info
    remote = strtok(NULL, "-");
    if (!remote) {
        printf("remote info is needed!\n\n");
        return -1;
    }

    // local info
    local = strtok(NULL, "-");
    if (!local) {
        printf("local info is needed!\n\n");
        return -1;
    }

    // param: remote_ip
    remote_ip = strtok(remote, ":");
    if (!remote_ip) {
        printf("remote info is invalid!\n\n");
        return -1;
    }

    if (!inet_aton(remote_ip, &addr)) {
        printf("remote_ip is invalid!\n\n");
        return -1;
    }
    flow->remote_ip = ntohl(addr.s_addr);

    // param: remote_port
    remote_port = strtok(NULL, ":");
    if (!remote_port) {
        printf("remote info is invalid!\n\n");
        return -1;
    }

    port = atoi(remote_port);
    if (port < 1 || port > 65535) {
        printf("remote port should in range (0,65536)!\n\n");
        return -1;
    }
    flow->remote_port = port;

    // param: remote_teid
    remote_teid = strtok(NULL, ":");
    if (!remote_teid) {
        printf("remote info is invalid!\n\n");
        return -1;
    }

    flow->remote_teid = (unsigned int)atoi(remote_teid);


    // param: local_ip
    local_ip = strtok(local, ":");
    if (!local_ip) {
        printf("local info is invalid!\n\n");
        return -1;
    }

    if (!inet_aton(local_ip, &addr)) {
        printf("local_ip is invalid!\n\n");
        return -1;
    }
    flow->local_ip = ntohl(addr.s_addr);

    // param: local_port
    local_port = strtok(NULL, ":");
    if (!local_port) {
        printf("local info is invalid!\n\n");
        return -1;
    }

    port = atoi(local_port);
    if (port < 1 || port > 65535) {
        printf("local port should in range (0,65536)!\n\n");
        return -1;
    }
    flow->local_port = port;

    // param: local_teid
    local_teid = strtok(NULL, ":");
    if (!local_teid) {
        printf("local info is invalid!\n\n");
        return -1;
    }
    flow->local_teid = (unsigned int)atoi(local_teid);

    flow->encap_bytes = 0;
    flow->decap_bytes = 0;
    flow->flags = 0;

    return 0;
}

int main(int argc, char **argv)
{
    struct gtp_flow flow;

    if (argc < 2) {
        show_usage();
        exit(0);
    }

    if (strcmp(argv[1], "show") == 0) {
        dump_flows();
    } else if (strcmp(argv[1], "add") == 0) {
        if (parse_flow_from_arg(argv[2], &flow)) {
            show_usage();
            exit(0);
        }

        if (add_del_flow(1, &flow)) {
            printf("add flow failed!\n");
        }
    } else if (strcmp(argv[1], "del") == 0) {
        if (parse_flow_from_arg(argv[2], &flow)) {
            show_usage();
            exit(0);
        }

        if (add_del_flow(0, &flow)) {
            printf("del flow failed!\n");
        }
    } else {
        show_usage();
    }
}
