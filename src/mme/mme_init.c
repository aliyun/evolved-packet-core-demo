#define TRACE_MODULE _mme_init

#include "core_debug.h"
#include "core_thread.h"
#include "core_msgq.h"
#include "core_fsm.h"

#include "gtp/gtp_xact.h"

#include "common/context.h"
#include "mme_event.h"

#include "mme_fd_path.h"
#include "s1ap_path.h"

#include "mme_sm.h"
#include <sys/socket.h>
#include <sys/un.h>
#include "core_epc_msg.h"

static thread_id sm_thread;
static void *THREAD_FUNC sm_main(thread_id id, void *data);

static thread_id net_thread;
static void *THREAD_FUNC net_main(thread_id id, void *data);

static int initialized = 0;

int g_mme_srv_fd = -1;
int g_mme_client_fd = -1;

status_t mme_initialize()
{
    status_t rv;

    if ((g_mme_srv_fd = epc_msg_server_init(EPC_SK_T_MME)) < 0) {
        d_error("Failed to init server fd!\n");
        return CORE_ERROR;
    }

    rv = mme_context_init();
    if (rv != CORE_OK) return rv;

    rv = mme_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = mme_context_setup_trace_module();
    if (rv != CORE_OK) return rv;

    rv = mme_m_tmsi_pool_generate();
    if (rv != CORE_OK) return rv;

    rv = mme_fd_init();
    if (rv != CORE_OK) return CORE_ERROR;

#define USRSCTP_LOCAL_UDP_PORT 9899
    rv = s1ap_init(context_self()->parameter.sctp_streams, USRSCTP_LOCAL_UDP_PORT);
    if (rv != CORE_OK) return rv;

    rv = thread_create(&sm_thread, NULL, sm_main, NULL);
    if (rv != CORE_OK) return rv;
    rv = thread_create(&net_thread, NULL, net_main, NULL);
    if (rv != CORE_OK) return rv;

    if ((g_mme_client_fd = epc_msg_client_init(EPC_SK_T_MME)) < 0) {
        d_error("MMe failed to init client fd!\n");
        return CORE_ERROR;
    }

    initialized = 1;

    return CORE_OK;
}

void mme_terminate(void)
{
    if (!initialized) return;

    thread_delete(net_thread);
    thread_delete(sm_thread);

    mme_fd_final();

    mme_context_final();

    s1ap_final();

    gtp_xact_final();
}

static void *THREAD_FUNC sm_main(thread_id id, void *data)
{
    event_t event;
    fsm_t mme_sm;
    c_time_t prev_tm, now_tm, prev_heart_beat_tm;
    //status_t rv;
    int rc;
    int max_fd;
    fd_set rset;
    struct timeval to;
    int conn_fd=-1, conn_fd1=-1, conn_fd2=-1;
    char buf[sizeof(EpcMsgHeader) + 1024] = {0};
    int n=0;
    EpcMsgHeader *msg = (EpcMsgHeader *)buf;
    
    max_fd = g_mme_srv_fd + 1;

    memset(&event, 0, sizeof(event_t));

    mme_self()->queue_id = event_create(MSGQ_O_BLOCK);
    d_assert(mme_self()->queue_id, return NULL, "MME event queue creation failed");
    tm_service_init(&mme_self()->tm_service, (expire_func_t)mme_event_timer_expire_func);
    gtp_xact_init(&mme_self()->tm_service, MME_EVT_S11_T3_RESPONSE, MME_EVT_S11_T3_HOLDING);

    fsm_create(&mme_sm, mme_state_initial, mme_state_final);
    fsm_init(&mme_sm, 0);

    prev_tm = time_now();
    prev_heart_beat_tm = time_now();    

#define EVENT_LOOP_TIMEOUT 50   /* 50ms */
    while ((!thread_should_stop()))
    {
#if 0
        rv = event_timedrecv(mme_self()->queue_id, &event, EVENT_LOOP_TIMEOUT*1000);

        d_assert(rv != CORE_ERROR, continue,
                "While receiving a event message, error occurs");

        now_tm = time_now();

        /* if the gap is over 10 ms, execute preriodic jobs */
        if (now_tm - prev_tm > EVENT_LOOP_TIMEOUT * 1000)
        {
            tm_execute_tm_service(&mme_self()->tm_service, mme_self()->queue_id);

            prev_tm = now_tm;
        }

        if (rv == CORE_TIMEUP)
        {
            continue;
        }

        fsm_dispatch(&mme_sm, (fsm_event_t*)&event);
#else
        FD_ZERO(&rset);
        FD_SET(g_mme_srv_fd, &rset);
        max_fd = g_mme_srv_fd + 1;
        if (conn_fd1 != -1)
        {
            FD_SET(conn_fd1, &rset);
            if (conn_fd1 > max_fd)
            {
                max_fd = conn_fd1 + 1;
            }
        }
        
        if (conn_fd2 != -1)
        {
            FD_SET(conn_fd2, &rset);
            if (conn_fd2 > max_fd)
            {
                max_fd = conn_fd2 + 1;
            }
        }
        to.tv_sec = 0;
        to.tv_usec = 50*1000;

        rc = select(max_fd, &rset, NULL, NULL, &to);
        
        now_tm = time_now();
        if (now_tm - prev_heart_beat_tm > (HEART_BEAT_INTERVAL * 1000 * 1000))
        {
            if (vnf_send_heart_beat("mme alive") == CORE_OK)
            {
                prev_heart_beat_tm = now_tm;
            }
        }

        if (rc < 0)
        {
            if (errno != EINTR && errno != 0)
                d_error("select failed(%d:%s)", errno, strerror(errno));

            continue;
        } else if (rc == 0) {
            /* if the gap is over 10 ms, execute preriodic jobs */
            if (now_tm - prev_tm > EVENT_LOOP_TIMEOUT * 1000)
            {
                tm_execute_tm_service(&mme_self()->tm_service, mme_self()->queue_id);
                prev_tm = now_tm;
            }
            continue;
        }

        if (FD_ISSET(g_mme_srv_fd, &rset))
        {
            struct sockaddr_un clientAddr;
            unsigned int sockAddrSize;

            sockAddrSize = sizeof(clientAddr);
            if ((conn_fd=accept(g_mme_srv_fd, (struct sockaddr *)&clientAddr, &sockAddrSize)) < 0) {
                d_error("srv fd accept failed! errno=%d", errno);
                continue;
            }

            if (conn_fd1 == -1)
            {
                conn_fd1 = conn_fd;
            }
            else
            {
                conn_fd2 = conn_fd;
                
                msg = (EpcMsgHeader *)buf;
                msg->dataLength = 1024;

                if ((n=epc_msg_recv(conn_fd2, &msg, NULL)) != 0) {
                    if (msg->dataLength > 1024) {
                        free((void*)msg);
                    }

                    if (n == 9002)
                    {
                        close(conn_fd2);
                        conn_fd2 = -1;
                    }
                    continue;
                }

                if (msg->type == MSG_T_EVENT_MME) {
                    fsm_dispatch(&mme_sm, (fsm_event_t*)(msg+1));
                }

                if (msg->dataLength > 1024) {
                    free((void*)msg);
                }
            }
        }
        else if (FD_ISSET(conn_fd1, &rset))
        {
            msg = (EpcMsgHeader *)buf;
            msg->dataLength = 1024;

            if ((n=epc_msg_recv(conn_fd1, &msg, NULL)) != 0) {
                d_error("epc_msg_receive failed! ret=%d", n);
                if (msg->dataLength > 1024) {
                    free((void*)msg);
                }

                if (n == 9002)
                {
                    close(conn_fd1);
                    conn_fd1 = -1;
                }
                continue;
            }

            if (msg->type == MSG_T_EVENT_MME) {
                fsm_dispatch(&mme_sm, (fsm_event_t*)(msg+1));
            }

            if (msg->dataLength > 1024) {
                free((void*)msg);
            }
        }
        else if (FD_ISSET(conn_fd2, &rset))
        {
            msg = (EpcMsgHeader *)buf;
            msg->dataLength = 1024;

            if ((n=epc_msg_recv(conn_fd2, &msg, NULL)) != 0) {
                //d_error("epc_msg_receive failed! ret=%d", n);
                if (msg->dataLength > 1024) {
                    free((void*)msg);
                }

                if (n == 9002)
                {
                    close(conn_fd2);
                    conn_fd2 = -1;
                }
                continue;
            }

            if (msg->type == MSG_T_EVENT_MME) {
                fsm_dispatch(&mme_sm, (fsm_event_t*)(msg+1));
            }

            if (msg->dataLength > 1024) {
                free((void*)msg);
            }
        }
#endif
    }

    fsm_final(&mme_sm, 0);
    fsm_clear(&mme_sm);

    event_delete(mme_self()->queue_id);

    return NULL;
}

static void *THREAD_FUNC net_main(thread_id id, void *data)
{
    while (!thread_should_stop())
    {
        sock_select_loop(EVENT_LOOP_TIMEOUT*1000); 
    }

    return NULL;
}
