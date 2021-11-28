#define TRACE_MODULE _ext_init

#include "core_debug.h"
#include "core_thread.h"
#include "ext_context.h"
#include "ext_msg_handler.h"
#include "ext_data_syn.h"

#include <sys/socket.h>
#include <sys/un.h>


static thread_id ext_thread;
static void *THREAD_FUNC ext_main(thread_id id, void *data);

static int initialized = 0;

status_t ext_initialize()
{
    status_t rv;

    rv = ext_context_init();
    if (rv != CORE_OK) return rv;

    rv = ext_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = ext_fds_init();
    if (rv != CORE_OK) return rv;

    rv = thread_create(&ext_thread, NULL, ext_main, NULL);
    if (rv != CORE_OK) return rv;

    rv = ext_context_setup_trace_module();
    if (rv != CORE_OK) return rv;
    
    initialized = 1;

    return CORE_OK;
}

void ext_terminate(void)
{
    if (!initialized) return;

    thread_delete(ext_thread);

    ext_fds_final();

    ext_context_final();
}

static void *THREAD_FUNC ext_main(thread_id id, void *data)
{
    int rc;
    c_int32_t max_fd;
    fd_set rset;
    struct timeval to;
    
    while ((!thread_should_stop())) {
        max_fd = ext_fill_fs_rset(&rset);

        to.tv_sec = 0;
        to.tv_usec = 50*1000;
        rc = select(max_fd, &rset, NULL, NULL, &to);

        //rc = select(max_fd, &rset, NULL, NULL, NULL);
        if (rc < 0) {
            if (errno != EINTR && errno != 0)
                d_error("select failed(%d:%s)", errno, strerror(errno));

            continue;
        }

        if (FD_ISSET(ext_self()->internal_fd, &rset)) {
            struct sockaddr_un clientAddr;
            unsigned int sockAddrSize;
            int conn_fd=-1;
            char buf[1024] = {0};
            int n=0;

            sockAddrSize = sizeof(clientAddr);
            if ((conn_fd=accept(ext_self()->internal_fd, (struct sockaddr *)&clientAddr, &sockAddrSize)) < 0) {
                d_error("ext srv fd accept failed! errno=%d\n", errno);
            }

            n = read(conn_fd, buf, sizeof(buf));
            if (n < 0) {
                d_error("ext srv read error! errno=%d\n", errno);
            }
            close(conn_fd);

            if (n < sizeof(EpcMsgHeader)) {
                continue;
            }

            epc_ext_msg_handle((EpcMsgHeader*)buf);
        }

        if (FD_ISSET(ext_self()->syn_fd, &rset)) {
            ext_data_handle_incoming_msg();
        }

    }

    return NULL;
}
