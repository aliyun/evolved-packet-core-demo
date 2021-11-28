#ifndef __UPF_CONTEXT_H__
#define __UPF_CONTEXT_H__

#include "core_list.h"
#include "core_index.h"
#include "core_errno.h"
#include "core_hash.h"
#include "core_network.h"
#include "core_msgq.h"
#include "core_timer.h"

#include "gtp/gtp_types.h"
#include "gtp/gtp_message.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_NUM_OF_DEV          16
#define MAX_NUM_OF_SUBNET       16

typedef struct _gtp_node_t gtp_node_t;
typedef struct _fd_config_t fd_config_t;

typedef struct _upf_context_t {
    c_uint32_t      gtpu_port;      /* Default: PGW GTP-U local port */
    list_t          gtpu_list;      /* PGW GTPU IPv4 Server List */
    list_t          gtpu_list6;     /* PGW GTPU IPv6 Server List */
    sock_id         gtpu_sock;      /* PGW GTPU IPv4 Socket */
    sock_id         gtpu_sock6;     /* PGW GTPU IPv6 Socket */
    c_sockaddr_t    *gtpu_addr;     /* PGW GTPU IPv4 Address */
    c_sockaddr_t    *gtpu_addr6;    /* PGW GTPU IPv6 Address */
    list_t          sgw_s5u_list;  /* SGW GTPU Node List */
} upf_context_t;


CORE_DECLARE(status_t)      upf_context_init(void);
CORE_DECLARE(status_t)      pgw_context_final(void);
CORE_DECLARE(upf_context_t*) upf_self(void);

CORE_DECLARE(status_t)      upf_context_parse_config(void);
CORE_DECLARE(status_t)      upf_context_setup_trace_module(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PGW_CONTEXT_H__ */
