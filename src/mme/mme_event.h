#ifndef __MME_EVENT_H__
#define __MME_EVENT_H__

#include "core_event.h"
#include "core_fsm.h"
#include "core_timer.h"
#include "core_pkbuf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* forward declaration */
typedef enum {
    MME_EVT_BASE = FSM_USER_SIG,

    MME_EVT_S1AP_MESSAGE,
    MME_EVT_S1AP_DELAYED_SEND,
    MME_EVT_S1AP_LO_ACCEPT,
    MME_EVT_S1AP_LO_SCTP_COMM_UP,
    MME_EVT_S1AP_LO_CONNREFUSED,
    MME_EVT_S1AP_S1_HOLDING_TIMER,

    MME_EVT_EMM_MESSAGE,
    MME_EVT_EMM_T3413,

    MME_EVT_ESM_MESSAGE,

    MME_EVT_S11_MESSAGE,
    MME_EVT_S11_T3_RESPONSE,
    MME_EVT_S11_T3_HOLDING,

    MME_EVT_S6A_MESSAGE,

    MME_EVT_TRIGGER_CFG_CHANGED = 100,

    MME_EVT_TOP,

} event_e;

//#define mme_event_send(__ptr_e) event_send(mme_self()->queue_id, (__ptr_e))

CORE_DECLARE(char*) mme_event_get_name(event_t *e);
CORE_DECLARE(status_t) mme_event_send(event_t *e);
CORE_DECLARE(void*) mme_event_timer_expire_func(c_uintptr_t queue_id, c_uintptr_t param1,
        c_uintptr_t param2, c_uintptr_t param3, c_uintptr_t param4,
        c_uintptr_t param5, c_uintptr_t param6);
CORE_DECLARE(tm_block_id) mme_event_timer_create(tm_service_t *tm_service, tm_type_e type, c_uint32_t duration, c_uintptr_t event);

#define mme_timer_create(__tm_service, __ptr_e, __duration) \
    mme_event_timer_create((__tm_service), TIMER_TYPE_ONE_SHOT, \
            (__duration), (__ptr_e))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MME_EVENT_H__ */
