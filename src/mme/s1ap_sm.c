#define TRACE_MODULE _s1ap_sm

#include "core_debug.h"

#include "nas/nas_message.h"
#include "gtp/gtp_message.h"

#include "s1ap_build.h"
#include "s1ap_handler.h"

#include "mme_event.h"
#include "mme_sm.h"

void s1ap_state_initial(fsm_t *s, event_t *e)
{
    d_assert(s, return, "Null param");

    mme_sm_trace(3, e);

    FSM_TRAN(s, &s1ap_state_operational);
}

void s1ap_state_final(fsm_t *s, event_t *e)
{
    d_assert(s, return, "Null param");

    mme_sm_trace(3, e);
}

void s1ap_state_operational(fsm_t *s, event_t *e)
{
    mme_enb_t *enb = NULL;

    d_assert(s, return, "Null param");
    d_assert(e, return, "Null param");

    mme_sm_trace(3, e);

    enb = mme_enb_find(event_get_param1(e));
    d_assert(enb, return,);

    switch (event_get(e))
    {
        case FSM_ENTRY_SIG:
        {
            break;
        }
        case FSM_EXIT_SIG:
        {
            break;
        }
        case MME_EVT_S1AP_MESSAGE:
        {
            S1AP_S1AP_PDU_t *pdu = (S1AP_S1AP_PDU_t *)event_get_param4(e);
            d_assert(pdu, break, "Null param");

            switch(pdu->present)
            {
                case S1AP_S1AP_PDU_PR_initiatingMessage :
                {
                    S1AP_InitiatingMessage_t *initiatingMessage = pdu->choice.initiatingMessage;
                    d_assert(initiatingMessage, break, "Null param");
                    
                    switch(initiatingMessage->procedureCode)
                    {
                        case S1AP_ProcedureCode_id_S1Setup :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_S1SetupRequest);
                            s1ap_handle_s1_setup_request(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_initialUEMessage :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_initialUEMessage);
                            s1ap_handle_initial_ue_message(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_uplinkNASTransport :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_uplinkNASTransport);
                            s1ap_handle_uplink_nas_transport(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_UECapabilityInfoIndication :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_UECapabilityInfoIndication);
                            s1ap_handle_ue_capability_info_indication(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_UEContextReleaseRequest:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_UEContextReleaseRequest);
                            s1ap_handle_ue_context_release_request(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_PathSwitchRequest:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_PathSwitchRequest);
                            s1ap_handle_path_switch_request(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_eNBConfigurationTransfer:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_eNBConfigurationTransfer);
                            pkbuf_t *pkbuf = (pkbuf_t *)event_get_param3(e);
                            d_assert(pkbuf, break, "Null param");

                            s1ap_handle_enb_configuration_transfer(enb, pdu, pkbuf);
                            break;
                        }
                        case S1AP_ProcedureCode_id_HandoverPreparation:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_HandoverRequired);
                            s1ap_handle_handover_required(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_HandoverCancel:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_HandoverCancel);
                            s1ap_handle_handover_cancel(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_eNBStatusTransfer:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_eNBStatusTransfer);
                            s1ap_handle_enb_status_transfer(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_HandoverNotification:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_HandoverNotification);
                            s1ap_handle_handover_notification(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_Reset:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_Reset);
                            s1ap_handle_s1_reset(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_E_RABModificationIndication:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_E_RABModificationIndication);
                            s1ap_handle_e_rab_modification_indication(enb, pdu);
                            break;
                        }
                        default:
                        {
                            d_warn("Not implemented(choice:%d, proc:%d)",
                                    pdu->present,
                                    initiatingMessage->procedureCode);
                            break;
                        }
                    }

                    break;
                }
                case S1AP_S1AP_PDU_PR_successfulOutcome :
                {
                    S1AP_SuccessfulOutcome_t *successfulOutcome = pdu->choice.successfulOutcome;
                    d_assert(successfulOutcome, break, "Null param");

                    switch(successfulOutcome->procedureCode)
                    {
                        case S1AP_ProcedureCode_id_InitialContextSetup :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_InitialContextSetupResponse);
                            s1ap_handle_initial_context_setup_response(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_E_RABSetup:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_E_RABSetupResponse);
                            s1ap_handle_e_rab_setup_response(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_E_RABModify:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_E_RABModifyResponse);
                            break;
                        }
                        case S1AP_ProcedureCode_id_E_RABRelease:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_E_RABReleaseResponse);
                            break;
                        }
                        case S1AP_ProcedureCode_id_UEContextRelease:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_UEContextReleaseComplete);
                            s1ap_handle_ue_context_release_complete(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_HandoverResourceAllocation:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_HandoverRequestAck);
                            s1ap_handle_handover_request_ack(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_WriteReplaceWarning:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_WriteReplaceWarningResponse);
                            s1ap_handle_write_replace_warning_response(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_Kill:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_KillResponse);
                            s1ap_handle_kill_response(enb, pdu);
                            break;
                        }
                        default:
                        {
                            d_warn("Not implemented(choice:%d, proc:%d)",
                                    pdu->present,
                                    successfulOutcome->procedureCode);
                            break;
                        }
                    }
                    break;
                }
                case S1AP_S1AP_PDU_PR_unsuccessfulOutcome :
                {
                    S1AP_UnsuccessfulOutcome_t *unsuccessfulOutcome = pdu->choice.unsuccessfulOutcome;
                    d_assert(unsuccessfulOutcome, break, "Null param");
                    
                    switch(unsuccessfulOutcome->procedureCode)
                    {
                        case S1AP_ProcedureCode_id_InitialContextSetup:
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_InitialContextSetupFailure);
                            s1ap_handle_initial_context_setup_failure(enb, pdu);
                            break;
                        }
                        case S1AP_ProcedureCode_id_HandoverResourceAllocation :
                        {
                            mme_inc_cnt_s1ap_in(CNT_S1AP_IN_HandoverFailure);
                            s1ap_handle_handover_failure(enb, pdu);
                            break;
                        }
                        default:
                        {
                            d_warn("Not implemented(choice:%d, proc:%d)",
                                    pdu->present,
                                    unsuccessfulOutcome->procedureCode);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    d_warn("Not implemented(choice:%d)", pdu->present);
                    break;
                }
            }

            break;
        }
        default:
        {
            d_error("Unknown event %s", mme_event_get_name(e));
            break;
        }
    }
}

void s1ap_state_exception(fsm_t *s, event_t *e)
{
    d_assert(s, return, "Null param");
    d_assert(e, return, "Null param");

    mme_sm_trace(3, e);

    switch (event_get(e))
    {
        case FSM_ENTRY_SIG:
        {
            break;
        }
        case FSM_EXIT_SIG:
        {
            break;
        }
        default:
        {
            d_error("Unknown event %s", mme_event_get_name(e));
            break;
        }
    }
}

