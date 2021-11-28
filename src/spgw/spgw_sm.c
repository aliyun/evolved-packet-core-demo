#define TRACE_MODULE _spgw_sm

#include "core_debug.h"
#include "core_lib.h"

#include "gtp/gtp_node.h"
#include "fd/fd_lib.h"
#include "fd/gx/gx_message.h"

#include "spgw_sm.h"
#include "spgw_context.h"
#include "spgw_event.h"
#include "spgw_gtp_path.h"
#include "pgw_s5c_handler.h"
#include "pgw_gx_handler.h"
#include "sgw_s11_handler.h"
#include "pgw_fd_path.h"

void spgw_state_initial(fsm_t *s, event_t *e)
{
    spgw_sm_trace(3, e);

    d_assert(s, return, "Null param");

    FSM_TRAN(s, &spgw_state_operational);
}

void spgw_state_final(fsm_t *s, event_t *e)
{
    spgw_sm_trace(3, e);

    d_assert(s, return, "Null param");
}

void spgw_state_operational(fsm_t *s, event_t *e)
{
    status_t rv;

    spgw_sm_trace(3, e);

    d_assert(s, return, "Null param");

    switch (event_get(e))
    {
        case FSM_ENTRY_SIG:
        {
            rv = pgw_gtp_open();
            if (rv != CORE_OK)
            {
                d_error("Can't establish PGW path");
                break;
            }
            break;
        }
        case FSM_EXIT_SIG:
        {
            rv = pgw_gtp_close();
            if (rv != CORE_OK)
            {
                d_error("Can't close PGW path");
                break;
            }
            break;
        }
        case SPGW_EVT_S11_MESSAGE:
        {
            status_t rv;
            pkbuf_t *pkbuf = (pkbuf_t *)event_get_param1(e);
            gtp_xact_t *xact = NULL;
            gtp_message_t message;
            sgw_ue_t *sgw_ue = NULL;

            d_assert(pkbuf, break,);
            rv = gtp_parse_msg(&message, pkbuf);
            d_assert(rv == CORE_OK, pkbuf_free(pkbuf); break,);

            if (message.h.teid == 0)
            {
                gtp_node_t *mme = spgw_mme_add_by_message(&message);
                d_assert(mme, pkbuf_free(pkbuf); break,);
                sgw_ue = spgw_ue_add_by_message(&message);
                SETUP_GTP_NODE(sgw_ue, mme);
            }
            else
            {
                sgw_ue = sgw_ue_find_by_teid(message.h.teid);
            }
            d_assert(sgw_ue, pkbuf_free(pkbuf); break,);

            rv = gtp_xact_receive(sgw_ue->gnode, &message.h, &xact);
            if (rv != CORE_OK)
            {
                pkbuf_free(pkbuf);
                break;
            }

            switch(message.h.type)
            {
                case GTP_CREATE_SESSION_REQUEST_TYPE:
                    sgw_s11_handle_create_session_request(xact, sgw_ue,
                            &message);
                    break;
                case GTP_MODIFY_BEARER_REQUEST_TYPE:
                    sgw_s11_handle_modify_bearer_request(xact, sgw_ue,
                            &message.modify_bearer_request);
                    break;
                case GTP_DELETE_SESSION_REQUEST_TYPE:
                    sgw_s11_handle_delete_session_request(xact, sgw_ue,
                            &message);
                    break;
                case GTP_CREATE_BEARER_RESPONSE_TYPE:
                    sgw_s11_handle_create_bearer_response(xact, sgw_ue,
                            &message);
                    break;
                case GTP_UPDATE_BEARER_RESPONSE_TYPE:
                    sgw_s11_handle_update_bearer_response(xact, sgw_ue,
                            &message);
                    break;
                case GTP_DELETE_BEARER_RESPONSE_TYPE:
                    sgw_s11_handle_delete_bearer_response(xact, sgw_ue,
                            &message);
                    break;
                case GTP_RELEASE_ACCESS_BEARERS_REQUEST_TYPE:
                    sgw_s11_handle_release_access_bearers_request(xact, sgw_ue,
                            &message.release_access_bearers_request);
                    break;
                case GTP_DOWNLINK_DATA_NOTIFICATION_ACKNOWLEDGE_TYPE:
                    sgw_s11_handle_downlink_data_notification_ack(xact, sgw_ue,
                            &message.downlink_data_notification_acknowledge);
                    break;
                case GTP_CREATE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
                    sgw_s11_handle_create_indirect_data_forwarding_tunnel_request(
                            xact, sgw_ue,
                            &message.
                            create_indirect_data_forwarding_tunnel_request);
                    break;
                case GTP_DELETE_INDIRECT_DATA_FORWARDING_TUNNEL_REQUEST_TYPE:
                    sgw_s11_handle_delete_indirect_data_forwarding_tunnel_request(
                            xact, sgw_ue);
                    break;
                default:
                    d_warn("Not implmeneted(type:%d)", message.h.type);
                    break;
            }
            pkbuf_free(pkbuf);
            break;
        }
        case SGW_EVT_S5C_MESSAGE:
        {
            status_t rv;
            pkbuf_t *pkbuf = (pkbuf_t *)event_get_param1(e);
            index_t xact_index = event_get_param2(e);
            gtp_xact_t *xact = NULL;
            gtp_message_t message;
            spgw_sess_t *sess = NULL;

            d_assert(pkbuf, break, "Null param");
            d_assert(xact_index, return, "Null param");

            rv = gtp_parse_msg(&message, pkbuf);
            d_assert(rv == CORE_OK, pkbuf_free(pkbuf); break,);

            sess = sgw_sess_find_by_teid(message.h.teid);
            d_assert(sess, pkbuf_free(pkbuf); break,);

#if 0
            rv = gtp_xact_receive(sess->gnode, &message.h, &xact);
            if (rv != CORE_OK)
            {
                pkbuf_free(pkbuf);
                break;
            }
#endif
            xact = gtp_xact_find(xact_index);
            d_assert(xact, return, "Null param");

            switch(message.h.type)
            {
                case GTP_CREATE_SESSION_RESPONSE_TYPE:
                    sgw_s5c_handle_create_session_response(xact, sess,
                            &message);
                    break;
                case GTP_DELETE_SESSION_RESPONSE_TYPE:
                    sgw_s5c_handle_delete_session_response(xact, sess,
                            &message);
                    break;
                case GTP_CREATE_BEARER_REQUEST_TYPE:
                    sgw_s5c_handle_create_bearer_request(xact, sess,
                            &message);
                    break;
                case GTP_UPDATE_BEARER_REQUEST_TYPE:
                    sgw_s5c_handle_update_bearer_request(xact, sess,
                            &message);
                    break;
                case GTP_DELETE_BEARER_REQUEST_TYPE:
                    sgw_s5c_handle_delete_bearer_request(xact, sess,
                            &message);
                    break;
                default:
                    d_warn("Not implmeneted(type:%d)", message.h.type);
                    break;
            }
            pkbuf_free(pkbuf);
            break;
        }        
        case PGW_EVT_S5C_MESSAGE:
        {
            status_t rv;
            pkbuf_t *recvbuf = (pkbuf_t *)event_get_param1(e);
            index_t xact_index = event_get_param2(e);
            spgw_sess_t *sess = (spgw_sess_t *)event_get_param3(e);
            pkbuf_t *copybuf = NULL;
            c_uint16_t copybuf_len = 0;
            gtp_xact_t *xact = NULL;
            gtp_message_t *message = NULL;

            d_assert(recvbuf, break, "Null param");
            d_assert(xact_index, return, "Null param");
            d_assert(sess, break, "Null param");

            copybuf_len = sizeof(gtp_message_t);
            copybuf = pkbuf_alloc(0, copybuf_len);
            d_assert(copybuf, break, "Null param");
            message = copybuf->payload;
            d_assert(message, break, "Null param");

            rv = gtp_parse_msg(message, recvbuf);
            d_assert(rv == CORE_OK, pkbuf_free(recvbuf); pkbuf_free(copybuf); break, "parse error");

            d_assert(sess, pkbuf_free(recvbuf); pkbuf_free(copybuf); break, "No Session Context");

            xact = gtp_xact_find(xact_index);
            d_assert(xact, return, "Null param");

            switch(message->h.type)
            {
                case GTP_CREATE_SESSION_REQUEST_TYPE:
                    pgw_s5c_handle_create_session_request(
                        sess, xact, &message->create_session_request);
                    pgw_gx_send_ccr(sess, xact, copybuf,
                        GX_CC_REQUEST_TYPE_INITIAL_REQUEST);
                    break;
                case GTP_DELETE_SESSION_REQUEST_TYPE:
                    pgw_s5c_handle_delete_session_request(
                        sess, xact, &message->delete_session_request);
                    pgw_gx_send_ccr(sess, xact, copybuf,
                        GX_CC_REQUEST_TYPE_TERMINATION_REQUEST);
                    break;
                case GTP_CREATE_BEARER_RESPONSE_TYPE:
                    pgw_s5c_handle_create_bearer_response(
                        sess, xact, &message->create_bearer_response);
                    pkbuf_free(copybuf);
                    break;
                case GTP_UPDATE_BEARER_RESPONSE_TYPE:
                    pgw_s5c_handle_update_bearer_response(
                        sess, xact, &message->update_bearer_response);
                    pkbuf_free(copybuf);
                    break;
                case GTP_DELETE_BEARER_RESPONSE_TYPE:
                    pgw_s5c_handle_delete_bearer_response(
                        sess, xact, &message->delete_bearer_response);
                    pkbuf_free(copybuf);
                    break;
                default:
                    d_warn("Not implmeneted(type:%d)", message->h.type);
                    pkbuf_free(copybuf);
                    break;
            }
            pkbuf_free(recvbuf);
            break;
        }
        case PGW_EVT_S5C_T3_RESPONSE:
        case PGW_EVT_S5C_T3_HOLDING:
        {
            gtp_xact_timeout(event_get_param1(e), event_get(e));
            break;
        }
        case PGW_EVT_GX_MESSAGE:
        {
            index_t sess_index = event_get_param1(e);
            spgw_sess_t *sess = NULL;
            pkbuf_t *gxbuf = (pkbuf_t *)event_get_param2(e);
            gx_message_t *gx_message = NULL;

            d_assert(sess_index, return, "Null param");
            sess = pgw_sess_find(sess_index);
            d_assert(sess, return, "Null param");

            d_assert(gxbuf, return, "Null param");
            gx_message = gxbuf->payload;
            d_assert(gx_message, return, "Null param");

            switch(gx_message->cmd_code)
            {
                case GX_CMD_CODE_CREDIT_CONTROL:
                {
                    index_t xact_index = event_get_param3(e);
                    gtp_xact_t *xact = NULL;

                    pkbuf_t *gtpbuf = (pkbuf_t *)event_get_param4(e);
                    gtp_message_t *message = NULL;

                    d_assert(xact_index, return, "Null param");
                    xact = gtp_xact_find(xact_index);
                    d_assert(xact, return, "Null param");

                    d_assert(gtpbuf, return, "Null param");
                    message = gtpbuf->payload;

                    if (gx_message->result_code != ER_DIAMETER_SUCCESS)
                    {
                        d_error("Diameter Error(%d)", gx_message->result_code);
                        break;
                    }
                    switch(gx_message->cc_request_type)
                    {
                        case GX_CC_REQUEST_TYPE_INITIAL_REQUEST:
                        {
                            pgw_gx_handle_cca_initial_request(
                                    sess, gx_message, xact, 
                                    &message->create_session_request);
                            break;
                        }
                        case GX_CC_REQUEST_TYPE_TERMINATION_REQUEST:
                        {
                            pgw_gx_handle_cca_termination_request(
                                    sess, gx_message, xact,
                                    &message->delete_session_request);
                            break;
                        }
                        default:
                        {
                            d_error("Not implemented(%d)", event_get_param4(e));
                            break;
                        }
                    }

                    pkbuf_free(gtpbuf);
                    break;
                }
                case GX_CMD_RE_AUTH:
                {
                    pgw_gx_handle_re_auth_request(sess, gx_message);
                    break;
                }
                default:
                {
                    d_error("Invalid type(%d)", event_get_param3(e));
                    break;
                }
            }

            gx_message_free(gx_message);
            pkbuf_free(gxbuf);
            break;
        }
        case SPGW_EVT_LO_DLDATA_NOTI:
        {
            index_t index = (index_t)event_get_param1(e);
            spgw_bearer_t* bearer = spgw_bearer_find(index);

            if (!bearer)
            {
                d_error("Can not find bearer with index(%d)",index);
                break;
            }

            sgw_s11_handle_lo_dldata_notification(bearer);

            break;
        }
        default:
        {
            d_error("No handler for event %s", spgw_event_get_name(e));
            break;
        }
    }
}
