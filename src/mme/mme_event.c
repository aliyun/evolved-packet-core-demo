#define TRACE_MODULE _mme_event

#include "core_debug.h"
#include "core_epc_msg.h"

#include "mme_event.h"

char* mme_event_get_name(event_t *e)
{
    if (e == NULL)
        return FSM_NAME_INIT_SIG;

    switch (event_get(e))
    {
        case FSM_ENTRY_SIG: 
            return FSM_NAME_ENTRY_SIG;
        case FSM_EXIT_SIG: 
            return FSM_NAME_EXIT_SIG;

        case MME_EVT_S1AP_MESSAGE:
            return "MME_EVT_S1AP_MESSAGE";
        case MME_EVT_S1AP_DELAYED_SEND:
            return "MME_EVT_S1AP_DELAYED_SEND";
        case MME_EVT_S1AP_LO_ACCEPT:
            return "MME_EVT_S1AP_LO_ACCEPT";
        case MME_EVT_S1AP_LO_SCTP_COMM_UP:
            return "MME_EVT_S1AP_LO_SCTP_COMM_UP";
        case MME_EVT_S1AP_LO_CONNREFUSED:
            return "MME_EVT_S1AP_LO_CONNREFUSED";
        case MME_EVT_S1AP_S1_HOLDING_TIMER:
            return "MME_EVT_S1AP_S1_HOLDING_TIMER";

        case MME_EVT_EMM_MESSAGE:
            return "MME_EVT_EMM_MESSAGE";
        case MME_EVT_EMM_T3413:
            return "MME_EVT_EMM_T3413";

        case MME_EVT_ESM_MESSAGE:
            return "MME_EVT_ESM_MESSAGE";

        case MME_EVT_S11_MESSAGE:
            return "MME_EVT_S11_MESSAGE";
        case MME_EVT_S11_T3_RESPONSE:
            return "MME_EVT_S11_T3_RESPONSE";
        case MME_EVT_S11_T3_HOLDING:
            return "MME_EVT_S11_T3_HOLDING";

        case MME_EVT_S6A_MESSAGE:
            return "MME_EVT_S6A_MESSAGE";

        case MME_EVT_TRIGGER_CFG_CHANGED:
            return "MME_EVT_TRIGGER_CFG_CHANGED";

        default: 
           break;
    }

    return EVT_NAME_UNKNOWN;
}

extern int g_mme_client_fd;
status_t mme_event_send(event_t *e)
{
    status_t rv=CORE_OK;
    char buf[sizeof(EpcMsgHeader)+sizeof(event_t)] = {0};
    EpcMsgHeader *msg = (EpcMsgHeader *)buf;

    msg->type = MSG_T_EVENT_MME;
    msg->dataLength = sizeof(event_t);
    memcpy((char *)(msg+1), (char *)e, sizeof(event_t));
    
    if (epc_msg_send(g_mme_client_fd, msg) != 0) {
        rv = CORE_ERROR;
    }

    return rv;
}

void* mme_event_timer_expire_func(c_uintptr_t queue_id, c_uintptr_t param1,
        c_uintptr_t param2, c_uintptr_t param3, c_uintptr_t param4,
        c_uintptr_t param5, c_uintptr_t param6)
{
    event_t e;
    status_t rv;

    d_trace(0, "%s Enter.\n", __FUNCTION__);

    d_assert(queue_id, return NULL, "Null param");
    event_set(&e, param1);
    event_set_param1(&e, param2);
    event_set_param2(&e, param3);
    event_set_param3(&e, param4);
    event_set_param4(&e, param5);
    event_set_param5(&e, param6);

    rv = mme_event_send(&e);
    if (rv != CORE_OK)
    {
        d_error("event_send error:%d", rv);
    } 

    return NULL;
}

tm_block_id mme_event_timer_create(tm_service_t *tm_service, tm_type_e type, 
        c_uint32_t duration, c_uintptr_t event)
{
    tm_block_id id;

    id = tm_create(tm_service,
            type, duration, (expire_func_t)mme_event_timer_expire_func);
    tm_set_param1(id, event);
    d_assert(id, return 0, "tm_create() failed");

    return id;
}
