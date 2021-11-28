#ifndef __APP_INIT_H__
#define __APP_INIT_H__

#include "core.h"
#include "core_errno.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) app_initialize(
        const char *config_path, const char *log_path, const char *pid_path);
CORE_DECLARE(void) app_terminate(void);

CORE_DECLARE(status_t) mme_initialize();
CORE_DECLARE(void) mme_terminate(void);

CORE_DECLARE(status_t) hss_initialize();
CORE_DECLARE(void) hss_terminate(void);

#if 0
CORE_DECLARE(status_t) sgw_initialize();
CORE_DECLARE(void) sgw_terminate(void);

CORE_DECLARE(status_t) pgw_initialize();
CORE_DECLARE(void) pgw_terminate(void);
#endif

CORE_DECLARE(status_t) pcrf_initialize();
CORE_DECLARE(void) pcrf_terminate(void);

CORE_DECLARE(status_t) ext_initialize();
CORE_DECLARE(void) ext_terminate(void);

CORE_DECLARE(status_t) spgw_initialize();
CORE_DECLARE(void) spgw_terminate(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_INIT_H__ */
