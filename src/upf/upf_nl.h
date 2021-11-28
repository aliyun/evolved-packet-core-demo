#ifndef __UPF_NL_H__
#define __UPF_NL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_FLOW_CNT_IN_MSG 20

typedef enum {
    GTP_MSG_RESP_OK=10,
    GTP_MSG_RESP_FAILED,
    GTP_MSG_ADD_FLOW,
    GTP_MSG_DEL_FLOW,
    GTP_MSG_DUMP_FLOW,
    GTP_MSG_GET_ALL_FLOW,
    GTP_MSG_GET_ALL_FLOW_RESP
} GtpMsgType;

/* all params using host byte order, not network byte order */
struct gtp_flow {
    unsigned int inner_ip;
    unsigned int remote_teid;
    unsigned int local_teid;
    unsigned int local_ip;
    unsigned short local_port;
    unsigned short remote_port;
    unsigned int remote_ip;
    unsigned int encap_bytes;
    unsigned int decap_bytes;
    unsigned int flags;
};

CORE_DECLARE(int) upf_add_flow(struct gtp_flow *flow);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
