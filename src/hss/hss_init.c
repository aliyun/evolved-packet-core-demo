#define TRACE_MODULE _hss_init

#include "core_debug.h"
#include "core_thread.h"
#include "hss_context.h"
#include "hss_fd_path.h"
#include "core_epc_msg.h"

static thread_id sm_thread;
static void *THREAD_FUNC sm_main(thread_id id, void *data);

static int initialized = 0;

status_t hss_initialize(void)
{
    status_t rv;

    rv = hss_context_init();
    if (rv != CORE_OK) return rv;

    rv = hss_context_parse_config();
    if (rv != CORE_OK) return rv;

    rv = hss_context_setup_trace_module();
    if (rv != CORE_OK) return rv;

    rv = hss_db_init();
    if (rv != CORE_OK) return rv;

    rv = hss_fd_init();
    if (rv != CORE_OK) return CORE_ERROR;

    rv = thread_create(&sm_thread, NULL, sm_main, NULL);
    if (rv != CORE_OK) return rv;

    initialized = 1;

	return CORE_OK;
}

void hss_terminate(void)
{
    if (!initialized) return;

    thread_delete(sm_thread);

    hss_fd_final();

    hss_db_final();
    hss_context_final();
	
	return;
}

static void *THREAD_FUNC sm_main(thread_id id, void *data)
{
    while (!thread_should_stop())
    {
        vnf_send_heart_beat("hss alive");
        sleep(HEART_BEAT_INTERVAL);
    }

    return NULL;
}
