#define TRACE_MODULE _pcrf_init

#include "core_debug.h"
#include "core_thread.h"
#include "pcrf_context.h"
#include "pcrf_fd_path.h"
#include "core_epc_msg.h"

static thread_id sm_thread;
static void *THREAD_FUNC sm_main(thread_id id, void *data);

static int initialized = 0;

status_t pcrf_initialize(void)
{
    status_t rv;

    rv = pcrf_context_init();
    if (rv != CORE_OK) return rv;

    rv = pcrf_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = pcrf_context_setup_trace_module();
    if (rv != CORE_OK) return rv;

    rv = pcrf_db_init();
    if (rv != CORE_OK) return rv;

    rv = pcrf_fd_init();
    if (rv != CORE_OK) return CORE_ERROR;

    rv = thread_create(&sm_thread, NULL, sm_main, NULL);
    if (rv != CORE_OK) return rv;

    initialized = 1;

	return CORE_OK;
}

void pcrf_terminate(void)
{
    if (!initialized) return;
    
    thread_delete(sm_thread);

    pcrf_fd_final();

    pcrf_db_final();
    pcrf_context_final();
	
	return;
}

static void *THREAD_FUNC sm_main(thread_id id, void *data)
{
    while (!thread_should_stop())
    {
        vnf_send_heart_beat("pcrf alive");
        sleep(HEART_BEAT_INTERVAL);
    }

    return NULL;
}
