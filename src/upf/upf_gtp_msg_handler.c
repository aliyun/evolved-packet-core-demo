#define TRACE_MODULE _upf_msg

#include "core_debug.h"
#include "core_thread.h"
#include "gtp/gtp_xact.h"
#include "upf_context.h"
#include "core_epc_msg.h"
#include "upf_nl.h"

#include <sys/socket.h>
#include <sys/un.h>

#define MAX_GTP_CTX_NUM 5
struct gtp_u_ctx g_gtp_ctx[MAX_GTP_CTX_NUM];

static void convert_imsi_to_string(char *dst, c_uint8_t *imsi, int len)
{
    int i = 0;

    for (i=0; i<len; i++) {
        sprintf(dst+2*i, "%x", *(imsi+i)&0xF);
        sprintf(dst+2*i+1, "%x", (*(imsi+i)&0xF0)>>4);
    }

    if (len == 8) {
        if ( *(dst+15) == 'f') {
            *(dst+15) = '\0';
        }
    }
}

status_t init_gtp_ctx_pool()
{
    memset(&g_gtp_ctx, 0, sizeof(struct gtp_u_ctx) * MAX_GTP_CTX_NUM);

    return CORE_OK;
}

static struct gtp_u_ctx* find_ctx(struct gtp_u_ctx* new)
{
    int i=0;

    for (i=0; i<MAX_GTP_CTX_NUM; i++) {
        if (g_gtp_ctx[i].half == 1 &&
                new->ebi == g_gtp_ctx[i].ebi &&
                new->imsi_len == g_gtp_ctx[i].imsi_len &&
                memcmp(new->imsi, g_gtp_ctx[i].imsi, new->imsi_len) == 0) {
            return &g_gtp_ctx[i];
        }
    }

    return NULL;
}

static struct gtp_u_ctx* get_available_node()
{
    int i=0;

    for (i=0; i<MAX_GTP_CTX_NUM; i++) {
        if (g_gtp_ctx[i].half == 0) {
            return &g_gtp_ctx[i];
        }
    }

    d_trace(1, "get available node failed!\n");
    return NULL;
}

static c_uint8_t check_ctx_info_completed(struct gtp_u_ctx* ctx)
{
    if (ctx->ebi == 0) {
        return 0;
    }
    if (ctx->ue_ip == 0) {
        return 0;
    }
    if (ctx->enb_ip == 0) {
        return 0;
    }
    if (ctx->epc_ip == 0) {
        return 0;
    }
    if (ctx->enb_s1u_teid == 0) {
        return 0;
    }
    if (ctx->sgw_s1u_teid == 0) {
        return 0;
    }
    if (ctx->sgw_s5u_teid == 0) {
        return 0;
    }
    if (ctx->pgw_s5u_teid == 0) {
        return 0;
    }

    return 1;
}

static void msg_handle_add_gtp_flow(struct gtp_u_ctx* ctx_node)
{
    struct gtp_flow flow;

    flow.inner_ip = ntohl(ctx_node->ue_ip);
    flow.remote_ip = ntohl(ctx_node->enb_ip);
    flow.local_ip = ntohl(ctx_node->epc_ip);
    flow.remote_port = 2152;
    flow.remote_teid = ctx_node->enb_s1u_teid;
    flow.local_port = ctx_node->epc_port;
    flow.local_teid = ctx_node->sgw_s1u_teid;

    if (upf_add_flow(&flow) < 0) {
        d_trace(1, "upf add flow failed!");
    }
}

status_t epc_msg_handle(EpcMsgHeader* msg)
{
    struct gtp_u_ctx *msg_info = NULL;
    struct gtp_u_ctx* ctx_node = NULL;
    char buf_ue[CORE_ADDRSTRLEN];
    char buf_enb[CORE_ADDRSTRLEN];
    char buf_epc[CORE_ADDRSTRLEN];
    char imsi[32] = {0};

    if (msg->type >= MSG_T_GTP_S1U_ADD && msg->type <= MSG_T_GTP_S5U_DEL) {
        msg_info = (struct gtp_u_ctx *)(msg + 1);
        convert_imsi_to_string(imsi, msg_info->imsi, msg_info->imsi_len);

        d_trace(1, "T:%d imsi:%s ebi:%d IPs:%s-%s-%s teids: 0x%x-0x%x-0x%x-0x%x\n",
                msg->type, imsi, msg_info->ebi,
                INET_NTOP(&msg_info->ue_ip, buf_ue),
                INET_NTOP(&msg_info->enb_ip, buf_enb),
                INET_NTOP(&msg_info->epc_ip, buf_epc),
                msg_info->enb_s1u_teid, msg_info->sgw_s1u_teid,
                msg_info->sgw_s5u_teid, msg_info->pgw_s5u_teid);
    }

    switch(msg->type) {
        case MSG_T_GTP_S1U_ADD:
            if (msg_info->ebi == 0) {
                d_error("Unknown msg type %d\n", msg->type);
                break;
            }
            ctx_node = find_ctx(msg_info);
            if (ctx_node == NULL) {
                ctx_node = get_available_node();
                if (ctx_node == NULL) {
                    d_error("can not get empty node!\n");
                    break;
                }
                memcpy(ctx_node->imsi, msg_info->imsi, msg_info->imsi_len);
                ctx_node->imsi_len = msg_info->imsi_len;
            }
            ctx_node->half = 1;
            ctx_node->ebi = msg_info->ebi;
            ctx_node->enb_s1u_teid = msg_info->enb_s1u_teid;
            ctx_node->sgw_s1u_teid = msg_info->sgw_s1u_teid;
            ctx_node->enb_ip = msg_info->enb_ip;
            ctx_node->epc_ip = msg_info->epc_ip;
            ctx_node->epc_port = msg_info->epc_port;
            if (check_ctx_info_completed(ctx_node) == 1) {
                d_trace(1, "CTX COMPLETED: imsi:%s ebi:%d IPs:%s-%s-%s teids: 0x%x-0x%x-0x%x-0x%x\n",
                        imsi, ctx_node->ebi,
                        INET_NTOP(&ctx_node->ue_ip, buf_ue),
                        INET_NTOP(&ctx_node->enb_ip, buf_enb),
                        INET_NTOP(&ctx_node->epc_ip, buf_epc),
                        ctx_node->enb_s1u_teid, ctx_node->sgw_s1u_teid,
                        ctx_node->sgw_s5u_teid, ctx_node->pgw_s5u_teid);

                msg_handle_add_gtp_flow(ctx_node);
                memset(ctx_node, 0, sizeof(struct gtp_u_ctx));
            }
            break;

        case MSG_T_GTP_S1U_DEL:
            break;

        case MSG_T_GTP_S5U_ADD:
            msg_info = (struct gtp_u_ctx *)(msg+1);

            if (msg_info->ebi == 0) {
                d_error("Unknown msg type %d\n", msg->type);
                break;
            }
            ctx_node = find_ctx(msg_info);
            if (ctx_node == NULL) {
                ctx_node = get_available_node();
                if (ctx_node == NULL) {
                    d_error("can not get empty node!\n");
                    break;
                }
                memcpy(ctx_node->imsi, msg_info->imsi, msg_info->imsi_len);
                ctx_node->imsi_len = msg_info->imsi_len;
            }
            ctx_node->half = 1;
            ctx_node->ebi = msg_info->ebi;
            ctx_node->ue_ip = msg_info->ue_ip;
            ctx_node->sgw_s5u_teid = msg_info->sgw_s5u_teid;
            ctx_node->pgw_s5u_teid = msg_info->pgw_s5u_teid;
            if (check_ctx_info_completed(ctx_node) == 1) {
                d_trace(1, "CTX COMPLETED: imsi:%s ebi:%d IPs:%s-%s-%s teids: 0x%x-0x%x-0x%x-0x%x\n",
                        imsi, ctx_node->ebi,
                        INET_NTOP(&ctx_node->ue_ip, buf_ue),
                        INET_NTOP(&ctx_node->enb_ip, buf_enb),
                        INET_NTOP(&ctx_node->epc_ip, buf_epc),
                        ctx_node->enb_s1u_teid, ctx_node->sgw_s1u_teid,
                        ctx_node->sgw_s5u_teid, ctx_node->pgw_s5u_teid);

                msg_handle_add_gtp_flow(ctx_node);
                memset(ctx_node, 0, sizeof(struct gtp_u_ctx));
            }
            break;

        case MSG_T_GTP_S5U_DEL:
            break;

        default:
            d_error("Unknown msg type %d\n", msg->type);
    }
    

    return 0;
}
