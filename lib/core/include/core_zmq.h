#ifndef __ZMQ_PATH_H__
#define __ZMQ_PATH_H__

#include "core_network.h"
#include "core_pkbuf.h"

#include <zmq.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define SBI_BY_PASS_GO              1
#define FEATURE_TEST                0 

#define ZMQ_SHORT_REQ_ID            1

#define MAX_PULLER_NUM              8
#define MAX_ZMQ_ADDR_LEN            64
#define MAX_ZMQ_MSG_ID_LEN          128
#ifdef ZMQ_SHORT_REQ_ID
#define MAX_ZMQ_MSG_ID_FMT          "%02x-%03x-%08lX"
#else
#define MAX_ZMQ_MSG_ID_FMT          "%04u-%05u-%04u-%05u-%04u-%016lX"
#endif
#define MAX_ZMQ_AUTH_CTX_ID_LEN     32
#define MAX_ZMQ_AUTH_CTX_ID_FMT     "%08X-%016lX"

#ifdef SECOND_AMF_FLAG
#define ZMQ_NGAP_IN_PUSH_PATH       "tcp://localhost:39422"
#define ZMQ_NGAP_IN_PULL_PATH       "tcp://localhost:39413"
#define ZMQ_NGAP_OUT_PUSH_PATH      "tcp://localhost:39424"
#define ZMQ_NGAP_OUT_PULL_PATH      "tcp://*:39424"
#else
#define ZMQ_NGAP_IN_PUSH_PATH       "tcp://localhost:39412"
#define ZMQ_NGAP_IN_PULL_PATH       "tcp://localhost:39413"
#define ZMQ_NGAP_OUT_PUSH_PATH      "tcp://localhost:39414"
#define ZMQ_NGAP_OUT_PULL_PATH      "tcp://*:39414"
#endif

#define ZMQ_EVENT_AMF_PATH          "inproc://amf_event"
#define ZMQ_EVENT_SMF_PATH          "inproc://smf_event"
#define ZMQ_EVENT_UPF_PATH          "inproc://upf_event"
#define ZMQ_EVENT_FRONT_PATH        "inproc://front_event"
#define ZMQ_EVENT_HASH_PATH         "inproc://hash_event"


//#define ZMQ_SBI_AMF_PULL_PATH       "tcp://*:39416"
//#define ZMQ_SBI_AMF_PUSH_PATH       "tcp://localhost:39416"
//#define ZMQ_SBI_SMF_PULL_PATH       "tcp://*:39417"
//#define ZMQ_SBI_SMF_PUSH_PATH       "tcp://localhost:39417"
//#define ZMQ_SBI_UDM_PULL_PATH       "tcp://*:39418"
//#define ZMQ_SBI_UDM_PUSH_PATH       "tcp://localhost:39418"

#define SBI_BIND_MODE 1

#ifdef SBI_BIND_MODE
#define ZMQ_SBI_AMF_PULL_PATH       "ipc://amf_sbi_srv"
#define ZMQ_SBI_AMF_PUSH_PATH       ZMQ_SBI_AMF_PULL_PATH
#define ZMQ_SBI_SMF_PULL_PATH       "ipc://smf_sbi_srv"
#define ZMQ_SBI_SMF_PUSH_PATH       ZMQ_SBI_SMF_PULL_PATH
#define ZMQ_SBI_PCF_PULL_PATH       "ipc://pcf_sbi_srv"
#define ZMQ_SBI_PCF_PUSH_PATH       ZMQ_SBI_PCF_PULL_PATH
#define ZMQ_SBI_UDM_PULL_PATH       "ipc://udm_sbi_srv"
#define ZMQ_SBI_UDM_PUSH_PATH       ZMQ_SBI_UDM_PULL_PATH
#define ZMQ_SBI_AUSF_PULL_PATH      "ipc://ausf_sbi_srv"
#define ZMQ_SBI_AUSF_PUSH_PATH      ZMQ_SBI_AUSF_PULL_PATH
#define ZMQ_SBI_NRF_PULL_PATH       "ipc://nrf_sbi_srv"
#define ZMQ_SBI_NRF_PUSH_PATH       ZMQ_SBI_NRF_PULL_PATH
#define ZMQ_SBI_NSSF_PULL_PATH       "ipc://nssf_sbi_srv"
#define ZMQ_SBI_NSSF_PUSH_PATH       ZMQ_SBI_NSSF_PULL_PATH
#define ZMQ_SBI_HASH_PULL_PATH       "ipc://hash_sbi_srv"
#define ZMQ_SBI_HASH_PUSH_PATH       ZMQ_SBI_HASH_PULL_PATH
#else
/* using C streamer */
#define ZMQ_SBI_AMF_PULL_PATH       "tcp://localhost:39511"
#define ZMQ_SBI_AMF_PUSH_PATH       "tcp://localhost:39512"
#define ZMQ_SBI_SMF_PULL_PATH       "tcp://localhost:39513"
#define ZMQ_SBI_SMF_PUSH_PATH       "tcp://localhost:39514"
#define ZMQ_SBI_PCF_PULL_PATH       "tcp://localhost:39515"
#define ZMQ_SBI_PCF_PUSH_PATH       "tcp://localhost:39516"
#define ZMQ_SBI_UDM_PULL_PATH       "tcp://localhost:39517"
#define ZMQ_SBI_UDM_PUSH_PATH       "tcp://localhost:39518"
#define ZMQ_SBI_NRF_PULL_PATH       "tcp://localhost:39519"
#define ZMQ_SBI_NRF_PUSH_PATH       "tcp://localhost:39520"
#endif

#define ZMQ_SBI_UPF_PULL_PATH       "tcp://*:39419"
#define ZMQ_SBI_CLI_PUSH_PATH       "tcp://localhost:39420"

#define AMF_ZMQ_SBI_OUT_PUSH_PATH   "tcp://localhost:39512"
#define SMF_ZMQ_SBI_OUT_PUSH_PATH   "tcp://localhost:39612"

/* SBI */
typedef enum {
    SBI_ENTITY_NONE=0,
    SBI_ENTITY_AMF,
    SBI_ENTITY_SMF,
    SBI_ENTITY_UDM,
    SBI_ENTITY_PCF,
    SBI_ENTITY_AUSF,
    SBI_ENTITY_NRF,
    SBI_ENTITY_NSSF,
    SBI_ENTITY_GO,
    SBI_ENTITY_AMF_GO,
    SBI_ENTITY_SMF_GO,
    SBI_ENTITY_AUSF_GO,
    SBI_ENTITY_UDM_GO,
    SBI_ENTITY_NRF_GO,
    SBI_ENTITY_HASH,
    SBI_ENTITY_UPF,
    SBI_ENTITY_CLI
} sbi_entity_type_e;

typedef enum {
    SBI_MSG_T_NONE=0,
    SBI_MSG_T_SMF_TO_AMF_N1N2MessageTransfer,                       // 1
    SBI_MSG_T_UDM_TO_AMF_UECM_DeregistrationNotification,           // 2
    SBI_MSG_T_EXT_TO_SMF_PDU_SESS_MODIFY,                           // 3
    SBI_MSG_T_EXT_TO_SMF_PDU_SESS_RELEASE,                          // 4
    SBI_MSG_T_UDM_TO_AMF_SDM_Notification_Request,                  // 5
    SBI_MSG_T_AMF_TO_UDM_SDM_Notification_Response,                 // 6
    SBI_MSG_T_UDM_TO_SMF_SDM_Notification_Request,                  // 7
    SBI_MSG_T_SMF_TO_UDM_SDM_Notification_Response,                 // 8
    SBI_MSG_T_EXT_TO_UDM_SDM_CHANGE,                                // 9

    // TODO: to be removed, no deregistration request but update with purgeflag=true
    SBI_MSG_T_AMF_TO_UDM_UECM_Deregistration_Request,               // 12
    SBI_MSG_T_UDM_TO_AMF_UECM_Deregistration_Response,              // 13

    /* AMF <-> PCF */
    SBI_MSG_T_AMF_TO_PCF_AMPolicyControl_Create_Request=20,
    SBI_MSG_T_PCF_TO_AMF_AMPolicyControl_Create_Response=21,
    SBI_MSG_T_AMF_TO_PCF_AMPolicyControl_Update_Request=22,
    SBI_MSG_T_PCF_TO_AMF_AMPolicyControl_Update_Response=23,
    SBI_MSG_T_AMF_TO_PCF_AMPolicyControl_Delete_Request=24,
    SBI_MSG_T_PCF_TO_AMF_AMPolicyControl_Delete_Response=25,
    SBI_MSG_T_PCF_TO_AMF_AMPolicyControl_UpdateNotify=26,
    SBI_MSG_T_AMF_TO_PCF_AMPolicyControl_UpdateNotify_Ack=27,

    SBI_MSG_T_AMF_TO_EXT_AmfEvent_Notify_UeState=30,
    SBI_MSG_T_AMF_TO_EXT_AmfEvent_Notify_ladn=31,

    /* AMF <-> SMF */
    SBI_MSG_T_AMF_TO_SMF_SM_CTX_Create_Request          = 60,
    SBI_MSG_T_SMF_TO_AMF_SM_CTX_Create_Response         = 61,
    SBI_MSG_T_AMF_TO_SMF_SM_CTX_Update_Request          = 62,
    SBI_MSG_T_SMF_TO_AMF_SM_CTX_Update_Response         = 63,
    SBI_MSG_T_AMF_TO_SMF_SM_CTX_Release_Request         = 64,
    SBI_MSG_T_SMF_TO_AMF_SM_CTX_Release_Response        = 65,
    SBI_MSG_T_AMF_TO_SMF_SM_CTX_Retrieve_Request        = 66,
    SBI_MSG_T_SMF_TO_AMF_SM_CTX_Retrieve_Response       = 67,
    SBI_MSG_T_SMF_TO_AMF_N1N2_Message_Transfer          = 68,
    SBI_MSG_T_SMF_TO_AMF_EventExposure_Subscribe        = 69,
    SBI_MSG_T_AMF_TO_SMF_EventExposure_Notify           = 70,
    SBI_MSG_T_SMF_TO_AMF_SM_CTX_Create_Error            = 71,
    SBI_MSG_T_AMF_TO_SMF_SM_CTX_Status_Notify_Response  = 72,

    /* AMF <-> UDM */ //TODO: to be cleared
    SBI_MSG_T_AMF_TO_UDM_AuthInfoRequest                = 75,
    SBI_MSG_T_UDM_TO_AMF_AuthInfoResponse               = 76,
    SBI_MSG_T_UDM_TO_AMF_UECM_DeregistrationNotificationRequest     = 83,
    SBI_MSG_T_AMF_TO_UDM_UECM_DeregistrationNotificationResponse    = 84,

    /* SMF <-> PCF */
    SBI_MSG_T_SMF_TO_PCF_SMPolicyControl_Create_Request     = 90,
    SBI_MSG_T_PCF_TO_SMF_SMPolicyControl_Create_Response    = 91,
    SBI_MSG_T_SMF_TO_PCF_SMPolicyControl_Update_Request     = 92,
    SBI_MSG_T_PCF_TO_SMF_SMPolicyControl_Update_Response    = 93,
    SBI_MSG_T_SMF_TO_PCF_SMPolicyControl_Delete_Request     = 94,
    SBI_MSG_T_PCF_TO_SMF_SMPolicyControl_Delete_Response    = 95,
    SBI_MSG_T_PCF_TO_SMF_SMPolicyControl_UpdateNotify       = 96,
    SBI_MSG_T_SMF_TO_PCF_SMPolicyControl_UpdateNotify_Ack   = 97,
    SBI_MSG_T_SMF_TO_PCF_CreditControl_Request              = 98,
    SBI_MSG_T_PCF_TO_SMF_CreditControl_Response             = 99,
    SBI_MSG_T_PCF_TO_SMF_Terminate_Notification             = 160,
    SBI_MSG_T_PCF_TO_SMF_Update_Notification                = 161,

    SBI_MSG_T_AUSF_TO_AMF_AuthInfoResponse                  = 100,

    SBI_MSG_T_EXT_TO_AMF_Test_Trigger                       = 101,
    SBI_MSG_T_EXT_TO_SMF_Test_Trigger                       = 102,
    SBI_MSG_T_EXT_TO_SMF_PFD_MGMT_START                     = 103,
    SBI_MSG_T_EXT_TO_UDM_Test_Trigger                       = 104,
    SBI_MSG_T_EXT_TO_PCF_Test_Trigger                       = 105,
    SBI_MSG_T_EXT_TO_NRF_Test_Trigger                       = 106,

    /* should not change these values start */
    SBI_MSG_T_REQ_C_TO_GO                                       = 110,
    SBI_MSG_T_REQ_GO_TO_C                                       = 111,
    SBI_MSG_T_RSP_C_TO_GO                                       = 112,
    SBI_MSG_T_RSP_GO_TO_C                                       = 113,
    /* should not change these values end */


    SBI_MSG_T_AMF_TO_AUSF_GetAuthInfo_Request                   = 120,
    SBI_MSG_T_AUSF_TO_AMF_GetAuthInfo_Response                  = 121,
    SBI_MSG_T_AMF_TO_AUSF_Authenticate_Request                  = 124,
    SBI_MSG_T_AUSF_TO_AMF_Authenticate_Response                 = 125,


    SBI_MSG_T_EXT_TO_UDM_UECM_DeregistrationNotification_trigger = 130,

    /* NEF <-> SMF */
    SBI_MSG_NEF_TO_SMF_QOS_CUSTOMIZE                            = 150,

    /* AMF/SMF/UDM/AUSM <-->NRF (Reserved:[180-200])*/
    SBI_MSG_T_SMF_TO_NRF_NFREGISTRATION                         = 180,
    SBI_MSG_T_SMF_TO_NRF_NFUPDATE                               = 181,
    SBI_MSG_T_SMF_TO_NRF_NFDEREGISTRATION                       = 182,
    SBI_MSG_T_AMF_TO_NRF_NFREGISTRATION                         = 183,
    SBI_MSG_T_AMF_TO_NRF_NFUPDATE                               = 184,
    SBI_MSG_T_AMF_TO_NRF_NFDEREGISTRATION                       = 185,
    SBI_MSG_T_UDM_TO_NRF_NFREGISTRATION                         = 186,
    SBI_MSG_T_UDM_TO_NRF_NFUPDATE                               = 187,
    SBI_MSG_T_UDM_TO_NRF_NFDEREGISTRATION                       = 188,

    SBI_MSG_T_SMF_TO_NRF_SUBSCRIPTION                           = 189,
    SBI_MSG_T_NRF_TO_NF_SUBSCRIPTION_NOTIFICATION               = 190,
    SBI_MSG_T_SMF_TO_NRF_DISCOVERY_REQ                          = 191,
    SBI_MSG_T_UDM_TO_NRF_SUBSCRIPTION                           = 192,
    SBI_MSG_T_UDM_TO_NRF_DISCOVERY_REQ                          = 193,

    /***************************************************
     * AUSF ID Scope: [200, 250)
     ***************************************************/
    SBI_MSG_T_AMF_TO_AUSF_UEAU_Auth_Request = 200,
    SBI_MSG_T_AUSF_TO_AMF_UEAU_Auth_Response,
    SBI_MSG_T_AMF_TO_AUSF_UEAU_5gAka_Request,
    SBI_MSG_T_AUSF_TO_AMF_UEAU_5gAka_Response,
    SBI_MSG_T_AMF_TO_AUSF_UEAU_EapSess_Request,
    SBI_MSG_T_AUSF_TO_AMF_UEAU_EapSess_Response,
    SBI_MSG_T_AMF_TO_AUSF_SoR_Request,
    SBI_MSG_T_AUSF_TO_AMF_SoR_Response,
    SBI_MSG_T_AMF_TO_AUSF_UPU_Request,
    SBI_MSG_T_AUSF_TO_AMF_UPU_Response,

    /***************************************************
     * UDM ID Scope: [250, 350)
     ***************************************************/
    /* AUSF <-> UDM */
    SBI_MSG_T_AUSF_TO_UDM_UEAU_GenAuth_Request = 250,
    SBI_MSG_T_UDM_TO_AUSF_UEAU_GenAuth_Response,
    /* AMF <-> UDM */
    SBI_MSG_T_AMF_TO_UDM_SDM_GetData_Request,
    SBI_MSG_T_UDM_TO_AMF_SDM_GetData_Response,
    SBI_MSG_T_AMF_TO_UDM_SDM_Subscribe_Request,
    SBI_MSG_T_UDM_TO_AMF_SDM_Subscribe_Response,
    SBI_MSG_T_AMF_TO_UDM_SDM_Modify_Request,
    SBI_MSG_T_UDM_TO_AMF_SDM_Modify_Response,
    SBI_MSG_T_AMF_TO_UDM_SDM_Unsubscribe_Request,
    SBI_MSG_T_UDM_TO_AMF_SDM_Unsubscribe_Response,
    SBI_MSG_T_AMF_TO_UDM_SDM_Info_Request,
    SBI_MSG_T_UDM_TO_AMF_SDM_Info_Response,
    SBI_MSG_T_AMF_TO_UDM_UECM_Registration_Request,
    SBI_MSG_T_UDM_TO_AMF_UECM_Registration_Response,
    SBI_MSG_T_AMF_TO_UDM_UECM_Update_Request,
    SBI_MSG_T_UDM_TO_AMF_UECM_Update_Response,
    SBI_MSG_T_AMF_TO_UDM_EE_Notification_Request,
    SBI_MSG_T_UDM_TO_AMF_EE_Notification_Response,
    /* SMF <-> UDM */
    SBI_MSG_T_SMF_TO_UDM_SDM_GetData_Request,
    SBI_MSG_T_UDM_TO_SMF_SDM_GetData_Response,
    SBI_MSG_T_SMF_TO_UDM_SDM_Subscribe_Request,
    SBI_MSG_T_UDM_TO_SMF_SDM_Subscribe_Response,
    SBI_MSG_T_SMF_TO_UDM_SDM_Modify_Request,
    SBI_MSG_T_UDM_TO_SMF_SDM_Modify_Response,
    SBI_MSG_T_SMF_TO_UDM_SDM_Unsubscribe_Request,
    SBI_MSG_T_UDM_TO_SMF_SDM_Unsubscribe_Response,
    SBI_MSG_T_SMF_TO_UDM_UECM_Registration_Request,
    SBI_MSG_T_UDM_TO_SMF_UECM_Registration_Response,
    SBI_MSG_T_SMF_TO_UDM_UECM_Deregistration_Request,
    SBI_MSG_T_UDM_TO_SMF_UECM_Deregistration_Response,


    /* FOR NSSF, range [300, 349]*/
    SBI_MSG_T_AMF_TO_NSSF_NSSFSELGET               = 300,
    SBI_MSG_T_NSSF_TO_AMF_NSSFSELGET_Response      = 301,

    /* FOR AMF <-> NRF, range [350, 399]*/
    SBI_MSG_T_AMF_TO_NRF_NFDiscovery_Request       = 350,
    SBI_MSG_T_NRF_TO_AMF_NFDiscovery_Response      = 351,
    SBI_MSG_T_AMF_TO_NRF_NFSubscription_Request    = 352,
    SBI_MSG_T_NRF_TO_AMF_NFSubscription_Response   = 352,

    /* FOR AMF <-> AMF, range [400, 449]*/
    SBI_MSG_T_AMF_TO_AMF_COMM_UeCtxCreate_Request  = 400,
    SBI_MSG_T_AMF_TO_AMF_COMM_UeCtxCreate_Response,
    SBI_MSG_T_AMF_TO_AMF_COMM_N2InfoNotify,
    SBI_MSG_T_AMF_TO_AMF_COMM_N2InfoNotifyAck,

    SBI_MSG_T_HASH_TO_AMF_NGAP_MSG            = 600,

    SBI_MSG_T_CLI_TO_UPF_REQ                  = 650,
    SBI_MSG_T_UPF_TO_CLI_RSP                  ,
    SBI_MSG_T_GTU_PATH_LOST                   ,
    SBI_MSG_T_GTU_PATH_RECOVER                ,
    SBI_MSG_T_UPF_ENABLE_FEATURE_FRRT         ,
    SBI_MSG_T_UPF_DISABLE_FEATURE_FRRT        ,
    SBI_MSG_T_UPF_SET_RELEASE_TIME            ,
    SBI_MSG_T_UPF_SET_HEARTBEAT_INTERVAL      ,
    SBI_MSG_T_UPF_RESTORE_HEARTBEAT_INTERVAL  ,
    SBI_MSG_T_UPF_SET_APPLICATION_RULE        ,
    SBI_MSG_T_UPF_SET_PREDEFINE_RULE          ,
    SBI_MSG_T_UPF_SET_NONGBR_RULE             ,
    SBI_MSG_T_UPF_GET_SESSION                 ,
    SBI_MSG_T_UPF_GET_MULTIPLE_SESSION        ,
    SBI_MSG_T_UPF_SEND_NODE_ASSOCIATE_REQUEST
} sbi_msg_type_e;

typedef struct _zmq_sbi_hdr_t {
    c_uint32_t      src_id;
    c_uint32_t      dst_id;
    c_uint32_t      msg_type;
    c_uint32_t      msg_len;
    c_uint32_t      seq;
    c_uint32_t      flag;
} zmq_sbi_hdr_t;

#define SBI_HDR_LEN     sizeof(zmq_sbi_hdr_t)

/* NGAP */
typedef struct _zmq_ngap_hdr {
    c_uint32_t      front_id;
    c_uint32_t      front_ran_id;
    c_uint32_t      seq;
    c_uint32_t      flag;
    c_uint16_t      stream_no;
} zmq_ngap_hdr;

typedef int (*zmq_puller_handler)(char *data, int len);

CORE_DECLARE(status_t)  zmq_context_init();
CORE_DECLARE(void)      zmq_context_final(void *context);

CORE_DECLARE(void *)    zmq_create_pusher(void *context, char *path);
CORE_DECLARE(void)      zmq_close_pusher_puller(void *pusher_puller);
CORE_DECLARE(status_t)  zmq_send_ngap(void *pusher, char *buf, int len, zmq_ngap_hdr *ngap_hdr);
CORE_DECLARE(status_t)  zmq_pusher_send_buf(void *pusher, char *buf, int len);

CORE_DECLARE(void *)    zmq_create_puller(void *context, char *path, int need_bind);
CORE_DECLARE(void *)    zmq_create_puller_recv_buffer(void *context, char *path, int need_bind, int buffer_size);

CORE_DECLARE(status_t)  zmq_puller_register(void *puller, zmq_puller_handler handler);
CORE_DECLARE(void)      zmq_poll_loop(long timeout);

CORE_DECLARE(status_t)  zmq_gen_uuid_for_msg(c_int8_t *uuid, sbi_entity_type_e src, c_uint16_t src_instance_id,
        sbi_entity_type_e dst, c_uint16_t dst_instance_id, sbi_msg_type_e msg_type, c_int64_t seq);
CORE_DECLARE(c_int64_t)  zmq_gen_seq_id(void);

CORE_DECLARE(status_t) zmq_set_msg_type_to_msg_uuid(c_int8_t *uuid, sbi_msg_type_e msg_type);
CORE_DECLARE(sbi_msg_type_e) zmq_get_msg_type_from_msg_uuid(c_int8_t *uuid);
CORE_DECLARE(c_int64_t) zmq_get_seq_from_msg_uuid(c_int8_t *uuid);
CORE_DECLARE(void) zmq_build_auth_ctx_id(c_uint8_t *auth_ctx_id, int instance_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ZMQ_PATH_H__ */
