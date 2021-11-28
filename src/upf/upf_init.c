#define TRACE_MODULE _upf_init

#include "core_debug.h"
#include "core_thread.h"
#include "gtp/gtp_xact.h"
#include "upf_context.h"
#include "upf_gtp_msg_handler.h"

#include <sys/socket.h>
#include <sys/un.h>
#include "upf_libgtpnl.h"


static thread_id upf_thread;
static void *THREAD_FUNC upf_main(thread_id id, void *data);

static int initialized = 0;

int g_srv_fd = -1;

status_t upf_initialize()
{
    status_t rv;

    if ((g_srv_fd = epc_msg_server_init(EPC_SK_T_UDF)) < 0) {
        d_error("Failed to init server fd!\n");
        return CORE_ERROR;
    }

    rv = thread_create(&upf_thread, NULL, upf_main, NULL);
    if (rv != CORE_OK) return rv;

    rv = upf_context_setup_trace_module();
    if (rv != CORE_OK) return rv;
    
    rv = init_gtp_ctx_pool();
    if (rv != CORE_OK) return rv;

    //rv = upf_libgtpnl_init();
    //if (rv != CORE_OK) return rv;

    initialized = 1;

    return CORE_OK;
}

void upf_terminate(void)
{
    if (!initialized) return;

    thread_delete(upf_thread);

    close(g_srv_fd);
}

static void *THREAD_FUNC upf_main(thread_id id, void *data)
{
    int rc;
    int max_fd;
    fd_set rset;
    //struct timeval to;
    //
    printf("Enter upf_main.\n");


    max_fd = g_srv_fd + 1;
    
    while ((!thread_should_stop())) {
        FD_ZERO(&rset);
        FD_SET(g_srv_fd, &rset);

        //to.tv_sec = 0;
        //to.tv_usec = 50*1000;

        //rc = select(max_fd, &rset, NULL, NULL, &to);
        rc = select(max_fd, &rset, NULL, NULL, NULL);
        if (rc < 0)
        {
            if (errno != EINTR && errno != 0)
                d_error("select failed(%d:%s)", errno, strerror(errno));

            continue;
        }

        if (FD_ISSET(g_srv_fd, &rset)) {
            struct sockaddr_un clientAddr;
            unsigned int sockAddrSize;
            int conn_fd=-1;
            char buf[1024] = {0};
            int n=0;

            sockAddrSize = sizeof(clientAddr);
            if ((conn_fd=accept(g_srv_fd, (struct sockaddr *)&clientAddr, &sockAddrSize)) < 0) {
                d_error("UPF srv fd accept failed! errno=%d\n", errno);
            }

            n = read(conn_fd, buf, sizeof(buf));
            if (n < 0) {
                d_error("UPF srv read error! errno=%d\n", errno);
            }
            close(conn_fd);

            if (n < sizeof(EpcMsgHeader)) {
                continue;
            }

            epc_msg_handle((EpcMsgHeader*)buf);

        }

    }

    return NULL;
}
