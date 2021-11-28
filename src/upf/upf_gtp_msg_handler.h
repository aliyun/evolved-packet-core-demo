#ifndef __UPF_MSG_HANDLER_H__
#define __UPF_MSG_HANDLER_H__

#include "core_epc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) epc_msg_handle(EpcMsgHeader* msg);
CORE_DECLARE(status_t) init_gtp_ctx_pool();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPF_MSG_HANDLER_H__ */
