#ifndef __EXT_DATA_SYN_H__
#define __EXT_DATA_SYN_H__

#include "core_epc_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    DATA_SYN_UNKNOWN=0,
    DATA_SYN_FLOW_CMD
} data_syn_type_e;

typedef struct _data_syn_t {
    data_syn_type_e syn_type;
    c_uint16_t      cmd_len;
    c_uint16_t      seq;
} data_syn_t;

CORE_DECLARE(status_t) ext_data_syn_init_server(void);
CORE_DECLARE(status_t) ext_data_syn_send_one_flow(char *flow_cmd);
CORE_DECLARE(status_t) ext_data_handle_incoming_msg(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
