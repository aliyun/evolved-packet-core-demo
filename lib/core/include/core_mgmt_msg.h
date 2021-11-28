#ifndef __MGMT_MSG_H__
#define __MGMT_MSG_H__

#include "core.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MGMT_SRV "127.0.0.1"
#define EPC_SW_VERSION "V1.0.0"

typedef enum {
    MGMT_MSG_T_EPC_ONLINE=1,
    MGMT_MSG_T_ENB_ONLINE=2,
    MGMT_MSG_T_ENB_OFFLINE=3,
    MGMT_MSG_T_UE_ONLINE=4,
    MGMT_MSG_T_UE_OFFLINE=5,
    MGMT_MSG_T_UE_LOCATION_UPDATE=6,
} MgmtMsgType;

CORE_DECLARE(void) mgmt_report_epc_online();
CORE_DECLARE(void) mgmt_report_enb_online(c_uint32_t enb_id, struct in_addr addr);
CORE_DECLARE(void) mgmt_report_enb_offline(c_uint32_t enb_id, struct in_addr addr);
CORE_DECLARE(void) mgmt_report_ue_online(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr);
CORE_DECLARE(void) mgmt_report_ue_offline(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr);
CORE_DECLARE(void) mgmt_report_ue_location_update(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MGMT_MSG_H__ */
