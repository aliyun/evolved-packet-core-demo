#define TRACE_MODULE _spgw_init

#include "core_debug.h"
#include "core_thread.h"

#include "gtp/gtp_xact.h"

#include "spgw_context.h"
#include "spgw_event.h"
#include "spgw_sm.h"

#include "pgw_fd_path.h"
#include "core_epc_msg.h"

static thread_id spgw_thread;
static void *THREAD_FUNC spgw_main(thread_id id, void *data);

static int initialized = 0;

status_t spgw_initialize()
{
    status_t rv;

    rv = spgw_context_init();
    if (rv != CORE_OK) return rv;

    rv = spgw_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = spgw_context_setup_trace_module();
    if (rv != CORE_OK) return rv;

    rv = pgw_ue_pool_generate();
    if (rv != CORE_OK) return rv;

    rv = pgw_fd_init();
    if (rv != 0) return CORE_ERROR;

    rv = thread_create(&spgw_thread, NULL, spgw_main, NULL);
    if (rv != CORE_OK) return rv;

    initialized = 1;

    return CORE_OK;
}

void spgw_terminate(void)
{
    if (!initialized) return;

    thread_delete(spgw_thread);

    pgw_fd_final();

    spgw_context_final();

    gtp_xact_final();
}

static void *THREAD_FUNC spgw_main(thread_id id, void *data)
{
    event_t event;
    fsm_t spgw_sm;
    c_time_t prev_tm, now_tm, prev_heart_beat_tm;
    status_t rv;


    //spgw_self()->queue_id = event_create(MSGQ_O_NONBLOCK);
    //d_assert(spgw_self()->queue_id, return NULL, "SPGW event queue creation failed");
    rv = event_init(spgw_self()->zmq_context, spgw_self()->event_addr, &spgw_self()->zmq_event_receiver, &spgw_self()->zmq_event_sender);
    d_assert(rv == CORE_OK, return NULL, "SPGW event init failed");

    //tm_service_init(&spgw_self()->tm_service, NULL);
    tm_service_init(&spgw_self()->tm_service, (expire_func_t)event_timer_expire_func_zmq);
    gtp_xact_init(&spgw_self()->tm_service, PGW_EVT_S5C_T3_RESPONSE, PGW_EVT_S5C_T3_HOLDING);

    fsm_create(&spgw_sm, spgw_state_initial, spgw_state_final);
    fsm_init(&spgw_sm, 0);

    prev_tm = time_now();
    prev_heart_beat_tm = time_now();

//#define EVENT_LOOP_TIMEOUT 10   /* 10ms */
#define EVENT_LOOP_TIMEOUT 2   /* 10ms */
    while ((!thread_should_stop()))
    {
        /* TODO: move sock receive to the other thread */
        sock_select_loop(EVENT_LOOP_TIMEOUT*1000);
        do
        {
            memset(&event, 0, sizeof(event_t));
            //rv = event_recv(spgw_self()->queue_id, &event);
            rv = event_recv_with_timeout(spgw_self()->zmq_event_receiver, &event, EVENT_LOOP_TIMEOUT);
            d_assert(rv != CORE_ERROR, continue, "While receiving a event message, error occurs");

            now_tm = time_now();

            /* if the gap is over event_loop timeout, execute preriodic jobs */
            if (now_tm - prev_tm > (EVENT_LOOP_TIMEOUT * 1000))
            {
                //tm_execute_tm_service(&spgw_self()->tm_service, spgw_self()->queue_id);
                tm_execute_tm_service(&spgw_self()->tm_service, (c_uintptr_t)spgw_self()->zmq_event_sender);
                prev_tm = now_tm;
            }
            
            /* 3s */
            if (now_tm - prev_heart_beat_tm > (HEART_BEAT_INTERVAL * 1000 * 1000))
            {
                if (vnf_send_heart_beat("spgw alive") == CORE_OK)
                {
                    prev_heart_beat_tm = now_tm;
                }
            }

            //if (rv == CORE_EAGAIN)
            if (rv == CORE_TIMEUP)
            {
                continue;
            }

            fsm_dispatch(&spgw_sm, (fsm_event_t*)&event);
        } while(rv == CORE_OK);
    }

    fsm_final(&spgw_sm, 0);
    fsm_clear(&spgw_sm);

    //event_delete(spgw_self()->queue_id);
    event_final(&spgw_self()->zmq_event_receiver, &spgw_self()->zmq_event_sender);    

    return NULL;
}
