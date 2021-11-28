#ifndef __EXT_MSG_HANDLER_H__
#define __EXT_MSG_HANDLER_H__

#include "core_epc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) epc_ext_msg_handle(EpcMsgHeader* msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EXT_MSG_HANDLER_H__ */
