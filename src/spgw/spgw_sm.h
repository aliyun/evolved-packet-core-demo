#ifndef __PGW_SM_H__
#define __PGW_SM_H__

#include "core_param.h"
#include "core_fsm.h"
#include "core_event.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void spgw_state_initial(fsm_t *s, event_t *e);
void spgw_state_final(fsm_t *s, event_t *e);
void spgw_state_operational(fsm_t *s, event_t *e);
void spgw_state_exception(fsm_t *s, event_t *e);

#define spgw_sm_print(__pe) \
    d_print("%s(): %s\n", __func__, spgw_event_get_name(__pe))

#define spgw_sm_trace(__l, __pe) \
    d_trace(__l, "%s(): %s\n", __func__, spgw_event_get_name(__pe))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__PGW_SM_H__ */
