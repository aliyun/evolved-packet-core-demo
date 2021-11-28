#ifndef __PGW_S5C_HANDLER_H__
#define __PGW_S5C_HANDLER_H__

#include "gtp/gtp_message.h"

#include "spgw_context.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CORE_DECLARE(void) pgw_s5c_handle_create_session_request(
        spgw_sess_t *sess, gtp_xact_t *xact, gtp_create_session_request_t *req);
CORE_DECLARE(void) pgw_s5c_handle_delete_session_request(
        spgw_sess_t *sess, gtp_xact_t *xact, gtp_delete_session_request_t *req);
CORE_DECLARE(void) pgw_s5c_handle_create_bearer_response(
        spgw_sess_t *sess, gtp_xact_t *xact, gtp_create_bearer_response_t *req);
CORE_DECLARE(void) pgw_s5c_handle_update_bearer_response(
        spgw_sess_t *sess, gtp_xact_t *xact, gtp_update_bearer_response_t *req);
CORE_DECLARE(void) pgw_s5c_handle_delete_bearer_response(
        spgw_sess_t *sess, gtp_xact_t *xact, gtp_delete_bearer_response_t *req);

CORE_DECLARE(void) sgw_s5c_handle_create_session_response(gtp_xact_t *s5c_xact,
        spgw_sess_t *sess, gtp_message_t *gtp_message);
CORE_DECLARE(void) sgw_s5c_handle_delete_session_response(gtp_xact_t *s5c_xact,
        spgw_sess_t *sess, gtp_message_t *gtp_message);
CORE_DECLARE(void) sgw_s5c_handle_create_bearer_request(gtp_xact_t *s5c_xact,
        spgw_sess_t *sess, gtp_message_t *gtp_message);
CORE_DECLARE(void) sgw_s5c_handle_update_bearer_request(gtp_xact_t *s5c_xact, 
        spgw_sess_t *sess, gtp_message_t *gtp_message);
CORE_DECLARE(void) sgw_s5c_handle_delete_bearer_request(gtp_xact_t *s5c_xact, 
        spgw_sess_t *sess, gtp_message_t *gtp_message);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PGW_S5C_HANDLER_H__ */
