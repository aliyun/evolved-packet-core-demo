#define TRACE_MODULE _spgw_main

#include "core_debug.h"
#include "core_signal.h"
#include "core_semaphore.h"

#include "common/context.h"
#include "common/application.h"

#include "app_init.h"

status_t app_initialize(
        const char *config_path, const char *log_path, const char *pid_path)
{
    status_t rv;
    int app = 0;

    rv = app_log_pid(pid_path, "spgw");
    if (rv != CORE_OK) return rv;

    rv = app_will_initialize(config_path, log_path);
    if (rv != CORE_OK) return rv;

    app = context_self()->logger.trace.app;
    if (app)
    {
        d_trace_level(&_spgw_main, app);
    }

    d_trace(1, "SPGW try to initialize\n");
    rv = spgw_initialize();
    if (rv != CORE_OK)
    {
        d_error("Failed to intialize SPGW");
        return rv;
    }
    d_trace(1, "SPGW initialize...done\n");

    rv = app_did_initialize();
    if (rv != CORE_OK) return rv;

    return CORE_OK;
}

void app_terminate(void)
{
    app_will_terminate();

    d_trace(1, "SPGW try to terminate\n");
    spgw_terminate();
    d_trace(1, "SPGW terminate...done\n");

    app_did_terminate();
}
