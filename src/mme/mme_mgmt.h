#ifndef __MME_MGMT_H__
#define __MME_MGMT_H__

#include "core_mgmt_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


CORE_DECLARE(void) send_ue_online_to_mgmt_agent(mme_ue_t *mme_ue);
CORE_DECLARE(void) send_ue_offline_to_mgmt_agent(mme_ue_t *mme_ue);
CORE_DECLARE(void) send_ue_location_update_to_mgmt_agent(mme_ue_t *mme_ue);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MME_MGMT_H__ */
