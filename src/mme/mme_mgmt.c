#define TRACE_MODULE _emm_mgmt

#include "core_debug.h"
#include "mme_context.h"
#include "core_mgmt_msg.h"
#include "core_epc_msg.h"

static int delete_gtp_flow_for_ue(c_uint32_t *ue_addr)
{
    char cmd[512] = {0}, ip_buf[20] = {0};

    /* only need ue_ip */
    inet_ntop(AF_INET, ue_addr, ip_buf, 20);
    snprintf(cmd, 512, "./gtp_flow del %s-10.10.10.10:2152:1234-10.10.10.11:2152:1234 &", ip_buf);
    d_trace(1, "%s\n", cmd);
    system(cmd);

    if (nofity_flow_info_to_ext_module(cmd) != 0) {
        d_trace(1, "spgw: notify (%s) to ext failed.\n", cmd);
    }    

    return 0;
}

void send_ue_online_to_mgmt_agent(mme_ue_t *mme_ue)
{
    mme_sess_t *sess=NULL;

    d_assert((mme_ue->imsi_len>0), return, "Null param");
    d_assert(mme_ue->enb_ue, return, "Null param");
    d_assert(mme_ue->enb_ue->enb, return, "Null param");


    sess = mme_sess_first(mme_ue);
    while (sess) {
        if (sess->pdn) {
            if (sess->pdn->paa.pdn_type == GTP_PDN_TYPE_IPV4) {
                mgmt_report_ue_online(mme_ue->imsi,
                                      mme_ue->imsi_len,
                                      mme_ue->enb_ue->enb->enb_id,
                                      &sess->pdn->paa.addr);
                return;
            }
        }
        sess = mme_sess_next(sess);
    }

    d_error("No ipv4 addr for ue!");
}

void send_ue_offline_to_mgmt_agent(mme_ue_t *mme_ue)
{
    mme_sess_t *sess=NULL;

    d_assert((mme_ue->imsi_len>0), return, "Null param");
    d_assert(mme_ue->enb_ue, return, "Null param");
    d_assert(mme_ue->enb_ue->enb, return, "Null param");


    sess = mme_sess_first(mme_ue);
    while (sess) {
        if (sess->pdn) {
            if (sess->pdn->paa.pdn_type == GTP_PDN_TYPE_IPV4) {
                delete_gtp_flow_for_ue(&sess->pdn->paa.addr);
                mgmt_report_ue_offline(mme_ue->imsi,
                                       mme_ue->imsi_len,
                                       mme_ue->enb_ue->enb->enb_id,
                                       &sess->pdn->paa.addr);
                return;
            }
        }
        sess = mme_sess_next(sess);
    }

    d_error("No ipv4 addr for ue!");
}

void send_ue_location_update_to_mgmt_agent(mme_ue_t *mme_ue)
{
    mme_sess_t *sess=NULL;

    d_assert((mme_ue->imsi_len>0), return, "Null param");
    d_assert(mme_ue->enb_ue, return, "Null param");
    d_assert(mme_ue->enb_ue->enb, return, "Null param");


    sess = mme_sess_first(mme_ue);
    while (sess) {
        if (sess->pdn) {
            if (sess->pdn->paa.pdn_type == GTP_PDN_TYPE_IPV4) {
                mgmt_report_ue_location_update(mme_ue->imsi,
                                               mme_ue->imsi_len,
                                               mme_ue->enb_ue->enb->enb_id,
                                               &sess->pdn->paa.addr);
                return;
            }
        }
        sess = mme_sess_next(sess);
    }

    d_error("No ipv4 addr for ue!");
}
