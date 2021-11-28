#define TRACE_MODULE _ext_msg

#include "core_debug.h"
#include "core_thread.h"

#include <sys/socket.h>
#include <sys/un.h>
#include "ext_context.h"
#include "ext_data_syn.h"

status_t ext_data_syn_init_server()
{
    int fd,ret;
    struct sockaddr_in addr;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        d_error("[EXT] create socket fail!\n");
        return CORE_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(ext_self()->syn_port);

    ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        d_error("[EXT] socket bind fail!\n");
        return CORE_ERROR;
    }

    ext_self()->syn_fd = fd;

    return CORE_OK;
}

status_t ext_data_syn_send_one_flow(char *flow_cmd)
{
    status_t rv=CORE_OK;
    int fd;
    socklen_t len;
    struct sockaddr_in srv_addr;
    char buf[1400] = {0};
    data_syn_t *msg_hdr = (data_syn_t*)&buf;
    int data_len=0;

    d_assert(flow_cmd, return CORE_ERROR, "null param.");
    data_len = strlen(flow_cmd);
    msg_hdr->syn_type = DATA_SYN_FLOW_CMD;
    msg_hdr->cmd_len = data_len+1;
    msg_hdr->seq = ext_self()->flow_cmd_seq++;
    memcpy(buf+sizeof(data_syn_t), flow_cmd, data_len+1);
    data_len = sizeof(data_syn_t) + msg_hdr->cmd_len;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        d_error("create client socket fail!\n");
        return CORE_ERROR;
    }

    len = sizeof(struct sockaddr_in);
    memset(&srv_addr, 0, len);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(ext_self()->syn_peer_ip);
    srv_addr.sin_port = htons(ext_self()->syn_port);
    
    if (data_len != sendto(fd, buf, data_len, 0, (struct sockaddr *)&srv_addr, len)) {
        d_error("[EXT] send flow cmd to %s failed", ext_self()->syn_peer_ip);
        rv = CORE_ERROR;
    }
    close(fd);
    return rv;
}

status_t ext_data_syn_recv_one_flow(char *buf)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int len=0;
    data_syn_t *msg_hdr = (data_syn_t*)buf;

    len = recvfrom(ext_self()->syn_fd, buf, 1400, 0, (struct sockaddr *)&addr, &addr_len);
    if (len != sizeof(data_syn_t) + msg_hdr->cmd_len) {
        d_error("[EXT] recv flow error. len=%d, msg_hdr->cmd_len=%d", len, msg_hdr->cmd_len);
        return CORE_ERROR;
    }

    return CORE_OK;
}

status_t ext_data_handle_incoming_msg()
{
    char buf[1400] = {0};
    status_t rv;
    
    rv = ext_data_syn_recv_one_flow(buf);
    d_assert(rv==CORE_OK, return rv, "recv flow info failed.");

    d_trace(1, "syn flow: %s\n", buf+sizeof(data_syn_t));
    system(buf+sizeof(data_syn_t));

    return rv;
}
