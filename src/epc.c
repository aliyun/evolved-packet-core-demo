#define TRACE_MODULE _epc_main

#include "core_general.h"
#include "core_debug.h"
#include "core_semaphore.h"

#include "common/context.h"
#include "common/application.h"

#include "app_init.h"

static semaphore_id ext_sem1 = 0;
static semaphore_id ext_sem2 = 0;

static semaphore_id pcrf_sem1 = 0;
static semaphore_id pcrf_sem2 = 0;

static semaphore_id hss_sem1 = 0;
static semaphore_id hss_sem2 = 0;

static semaphore_id spgw_sem1 = 0;
static semaphore_id spgw_sem2 = 0;

status_t app_initialize(const char *config_path, const char *log_path, const char *pid_path)
{
    pid_t pid;
    status_t rv;
    int app = 0;

    rv = app_log_pid(pid_path, "epc");
    if (rv != CORE_OK) return rv;

    rv = app_will_initialize(config_path, log_path);
    if (rv != CORE_OK) return rv;

    app = context_self()->logger.trace.app;
    if (app)
    {
        d_trace_level(&_epc_main, app);
    }

    /************************* EXT Process **********************/
    semaphore_create(&ext_sem1, 0); /* copied to EXT/PCRF/SPGW/HSS process */
    semaphore_create(&ext_sem2, 0); /* copied to EXT/PCRF/SPGW/HSS process */

    if (context_self()->parameter.no_ext == 0)
    {
        pid = fork();
        d_assert(pid >= 0, _exit(EXIT_FAILURE), "fork() failed");

        if (pid == 0)
        {
            d_trace(1, "EXT init\n");
            rv = ext_initialize();
            d_assert(rv == CORE_OK,, "Failed to init EXT");
            d_trace(1, "EXT init done\n");

            if (ext_sem1) semaphore_post(ext_sem1);
            if (ext_sem2) semaphore_wait(ext_sem2);

            if (rv == CORE_OK)
            {
                d_trace(1, "EXT terminate\n");
                ext_terminate();
                d_trace(1, "EXT terminate done\n");
            }

            if (ext_sem1) semaphore_post(ext_sem1);

            /* allocated from parent process */
            if (ext_sem1) semaphore_delete(ext_sem1);
            if (ext_sem2) semaphore_delete(ext_sem2);

            app_did_terminate();

            core_terminate();

            _exit(EXIT_SUCCESS);
        }

        if (ext_sem1) semaphore_wait(ext_sem1);
    }

    /************************* PCRF Process **********************/
    semaphore_create(&pcrf_sem1, 0); /* copied to PCRF/SPGW/HSS process */
    semaphore_create(&pcrf_sem2, 0); /* copied to PCRF/SPGW/HSS process */

    if (context_self()->parameter.no_pcrf == 0)
    {
        pid = fork();
        d_assert(pid >= 0, _exit(EXIT_FAILURE), "fork() failed");

        if (pid == 0)
        {
            /* allocated from parent process */
            if (ext_sem1) semaphore_delete(ext_sem1);
            if (ext_sem2) semaphore_delete(ext_sem2);

            d_trace(1, "PCRF init\n");
            rv = pcrf_initialize();
            d_assert(rv == CORE_OK,, "Failed to init PCRF");
            d_trace(1, "PCRF init done\n");

            if (pcrf_sem1) semaphore_post(pcrf_sem1);
            if (pcrf_sem2) semaphore_wait(pcrf_sem2);

            if (rv == CORE_OK)
            {
                d_trace(1, "PCRF terminate\n");
                pcrf_terminate();
                d_trace(1, "PCRF terminate done\n");
            }

            if (pcrf_sem1) semaphore_post(pcrf_sem1);

            /* allocated from parent process */
            if (pcrf_sem1) semaphore_delete(pcrf_sem1);
            if (pcrf_sem2) semaphore_delete(pcrf_sem2);

            app_did_terminate();

            core_terminate();

            _exit(EXIT_SUCCESS);
        }

        if (pcrf_sem1) semaphore_wait(pcrf_sem1);
    }


    /************************* SPGW Process **********************/
    semaphore_create(&spgw_sem1, 0); /* copied to SPGW/HSS process */
    semaphore_create(&spgw_sem2, 0); /* copied to SPGW/HSS process */

    if (context_self()->parameter.no_spgw == 0)
    {
        pid = fork();
        d_assert(pid >= 0, _exit(EXIT_FAILURE), "fork() failed");

        if (pid == 0)
        {
            /* allocated from parent process */
            if (ext_sem1) semaphore_delete(ext_sem1);
            if (ext_sem2) semaphore_delete(ext_sem2);
            if (pcrf_sem1) semaphore_delete(pcrf_sem1);
            if (pcrf_sem2) semaphore_delete(pcrf_sem2);

            d_trace(1, "SPGW init\n");
            rv = spgw_initialize();
            d_assert(rv == CORE_OK,, "Failed to init SPGW");
            d_trace(1, "SPGW init done\n");

            if (spgw_sem1) semaphore_post(spgw_sem1);
            if (spgw_sem2) semaphore_wait(spgw_sem2);

            if (rv == CORE_OK)
            {
                d_trace(1, "SPGW terminate\n");
                spgw_terminate();
                d_trace(1, "SPGW terminate done\n");
            }

            if (spgw_sem1) semaphore_post(spgw_sem1);

            /* allocated from parent process */
            if (spgw_sem1) semaphore_delete(spgw_sem1);
            if (spgw_sem2) semaphore_delete(spgw_sem2);

            app_did_terminate();

            core_terminate();

            _exit(EXIT_SUCCESS);
        }

        if (spgw_sem1) semaphore_wait(spgw_sem1);
    }


    /************************* HSS Process **********************/
    semaphore_create(&hss_sem1, 0); /* copied to HSS process */
    semaphore_create(&hss_sem2, 0); /* copied to HSS process */

    if (context_self()->parameter.no_hss == 0)
    {
        pid = fork();
        d_assert(pid >= 0, _exit(EXIT_FAILURE), "fork() failed");

        if (pid == 0)
        {
            /* allocated from parent process */
            if (ext_sem1) semaphore_delete(ext_sem1);
            if (ext_sem2) semaphore_delete(ext_sem2);
            if (pcrf_sem1) semaphore_delete(pcrf_sem1);
            if (pcrf_sem2) semaphore_delete(pcrf_sem2);
            if (spgw_sem1) semaphore_delete(spgw_sem1);
            if (spgw_sem2) semaphore_delete(spgw_sem2);

            d_trace(1, "HSS init\n");
            rv = hss_initialize();
            d_assert(rv == CORE_OK,, "Failed to init HSS");
            d_trace(1, "HSS init done\n");

            if (hss_sem1) semaphore_post(hss_sem1);
            if (hss_sem2) semaphore_wait(hss_sem2);

            if (rv == CORE_OK)
            {
                d_trace(1, "HSS terminate\n");
                hss_terminate();
                d_trace(1, "HSS terminate done\n");
            }

            if (hss_sem1) semaphore_post(hss_sem1);

            if (hss_sem1) semaphore_delete(hss_sem1);
            if (hss_sem2) semaphore_delete(hss_sem2);

            app_did_terminate();

            core_terminate();

            _exit(EXIT_SUCCESS);
        }

        if (hss_sem1) semaphore_wait(hss_sem1);
    }

    rv = app_did_initialize();
    if (rv != CORE_OK) return rv;

    d_trace(1, "MME init\n");
    rv = mme_initialize();
    d_assert(rv == CORE_OK, return rv, "Failed to init MME");
    d_trace(1, "MME init done\n");

    return CORE_OK;;
}

void app_terminate(void)
{
    app_will_terminate();

    d_trace(1, "MME terminate\n");
    mme_terminate();
    d_trace(1, "MME terminate done\n");

    if (context_self()->parameter.no_hss == 0)
    {
        if (hss_sem2) semaphore_post(hss_sem2);
        if (hss_sem1) semaphore_wait(hss_sem1);
    }
    if (hss_sem1) semaphore_delete(hss_sem1);
    if (hss_sem2) semaphore_delete(hss_sem2);

    if (context_self()->parameter.no_spgw == 0)
    {
        if (spgw_sem2) semaphore_post(spgw_sem2);
        if (spgw_sem1) semaphore_wait(spgw_sem1);
    }
    if (spgw_sem1) semaphore_delete(spgw_sem1);
    if (spgw_sem2) semaphore_delete(spgw_sem2);

    if (context_self()->parameter.no_pcrf == 0)
    {
        if (pcrf_sem2) semaphore_post(pcrf_sem2);
        if (pcrf_sem1) semaphore_wait(pcrf_sem1);
    }
    if (pcrf_sem1) semaphore_delete(pcrf_sem1);
    if (pcrf_sem2) semaphore_delete(pcrf_sem2);

    if (context_self()->parameter.no_ext == 0)
    {
        if (ext_sem2) semaphore_post(ext_sem2);
        if (ext_sem1) semaphore_wait(ext_sem1);
    }
    if (ext_sem1) semaphore_delete(ext_sem1);
    if (ext_sem2) semaphore_delete(ext_sem2);

    app_did_terminate();
}
