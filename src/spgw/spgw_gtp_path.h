#ifndef __PGW_PATH_H__
#define __PGW_PATH_H__

#include "gtp/gtp_xact.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(status_t) pgw_gtp_open();
CORE_DECLARE(status_t) pgw_gtp_close();
CORE_DECLARE(status_t) sgw_gtp_send_end_marker(sgw_tunnel_t *s1u_tunnel);
CORE_DECLARE(status_t) pgw_update_s5u_gtp(int action,
                                          c_uint8_t *imsi,
                                          int imsi_len,
                                          c_uint8_t ebi,
                                          c_uint32_t ue_ip,
                                          c_uint32_t sgw_s5u_teid,
                                          c_uint32_t pgw_s5u_teid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PGW_PATH_H__ */
