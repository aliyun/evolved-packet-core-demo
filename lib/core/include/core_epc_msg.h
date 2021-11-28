#ifndef __EPC_MSG_H__
#define __EPC_MSG_H__

#include "core_event.h"
#include "3gpp_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MSECS_IN_SEC 1000
#define USECS_IN_MSEC 1000

#define HEART_BEAT_INTERVAL 3   //second 
#define HEART_BEAT_SRV_ADDR "127.0.0.1"
#define HEART_BEAT_SRV_PORT 9998


typedef enum {
    EPC_SK_T_UDF,
    EPC_SK_T_MME,
    EPC_SK_T_EXT
} EpcMsgSockType;

typedef enum {
    EID_UDF,
    EID_MME,
    EID_EXT
} EpcMsgEid;

typedef enum {
    MSG_T_EVENT = 1,
    MSG_T_EVENT_MME,
    MSG_T_GTP_S1U_ADD,
    MSG_T_GTP_S1U_DEL,
    MSG_T_GTP_S5U_ADD,
    MSG_T_GTP_S5U_DEL,
    MSG_T_GTP_FLOW_SYN
} EpcMsgType;

struct gtp_u_ctx {
    c_uint8_t  imsi[MAX_IMSI_LEN];
    int        imsi_len;
    c_uint32_t ue_ip;
    c_uint32_t enb_ip;
    c_uint32_t epc_ip;
    c_uint32_t enb_s1u_teid;
    c_uint32_t sgw_s1u_teid;
    c_uint32_t sgw_s5u_teid;
    c_uint32_t pgw_s5u_teid;
    c_uint16_t epc_port;
    c_uint8_t  ebi;
ED4(c_uint8_t  ipv4:1;,
    c_uint8_t  ipv6:1;,
    c_uint8_t  half:1;,
    c_uint8_t  reserved:5;)
};

typedef struct {
    EpcMsgType type;
    //event_t event;
    unsigned int dataLength;
} EpcMsgHeader;

CORE_DECLARE(int) epc_msg_server_init(EpcMsgSockType type);
CORE_DECLARE(int) epc_msg_client_init(EpcMsgSockType type);
CORE_DECLARE(int) epc_msg_recv(int fd, EpcMsgHeader **msg, unsigned int *timeout);
CORE_DECLARE(int) epc_msg_send(int fd, const EpcMsgHeader *msg);
CORE_DECLARE(int) epc_msg_send_once(EpcMsgEid dst, const EpcMsgHeader *msg);

CORE_DECLARE(int) nofity_flow_info_to_ext_module(const char* cmd);
CORE_DECLARE(status_t) vnf_send_heart_beat(c_int8_t *msg);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TLV_MSG_H__ */
