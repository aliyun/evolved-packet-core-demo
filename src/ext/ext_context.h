#ifndef __EXT_CONTEXT_H__
#define __EXT_CONTEXT_H__

#include "core_list.h"
#include "core_index.h"
#include "core_errno.h"
#include "core_hash.h"
#include "core_network.h"
#include "core_msgq.h"
#include "core_timer.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EXT_DATA_SYN_PORT 2162

typedef struct _ext_context_t {
    c_int32_t       internal_fd;
    const char      *syn_peer_ip;
    c_uint32_t      syn_port;
    c_int32_t       syn_fd;
    c_uint16_t      flow_cmd_seq;
} ext_context_t;


CORE_DECLARE(status_t)      ext_context_init(void);
CORE_DECLARE(status_t)      ext_context_final(void);
CORE_DECLARE(ext_context_t*) ext_self(void);

CORE_DECLARE(status_t)      ext_context_parse_config(void);
CORE_DECLARE(status_t)      ext_context_setup_trace_module(void);

CORE_DECLARE(status_t)      ext_fds_init(void);
CORE_DECLARE(void)          ext_fds_final(void);
CORE_DECLARE(c_int32_t)     ext_fill_fs_rset(fd_set *rset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EXT_CONTEXT_H__ */
