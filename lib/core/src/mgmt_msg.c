#include "core_mgmt_msg.h"

#define MGMT_DEBUG 1

#ifdef MGMT_DEBUG
#define MGMT_PRINT(...) printf(__VA_ARGS__)
#else
#define MGMT_PRINT(...)
#endif

#define AGENT_C_PATH "/epc/bin"

void mgmt_report_epc_online(c_uint32_t enb_id, struct in_addr addr)
{
    char cmd[512] = {0};

    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"softVer\":\"%s\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_EPC_ONLINE,
                       EPC_SW_VERSION);
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}

void mgmt_report_enb_online(c_uint32_t enb_id, struct in_addr addr)
{
    char cmd[512] = {0};

    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"enbID\":\"%d\",\"ip\":\"%s\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_ENB_ONLINE,
                       enb_id,
                       inet_ntoa(addr));
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}

void mgmt_report_enb_offline(c_uint32_t enb_id, struct in_addr addr)
{
    char cmd[512] = {0};

    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"enbID\":\"%d\",\"ip\":\"%s\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_ENB_OFFLINE,
                       enb_id,
                       inet_ntoa(addr));
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}

void mgmt_report_ue_online(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr)
{
    char cmd[512]={0}, imsi_str[64]={0}, ip_buf[20]={0};
    int i;

    for (i=0; i<imsi_len; i++) {
        sprintf(imsi_str+2*i, "%x", *(imsi+i)&0xF);
        sprintf(imsi_str+2*i+1, "%x", (*(imsi+i)&0xF0)>>4);
    }

    if (imsi_len == 8) {
        if (imsi_str[15] == 'f') {
            imsi_str[15] = '\0';
        }
    }

    inet_ntop(AF_INET, addr, ip_buf, 20);
    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"imsi\":\"%s\",\"ip\":\"%s\",\"enbid\":\"%d\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_UE_ONLINE,
                       imsi_str,
                       ip_buf,
                       enb_id);
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}

void mgmt_report_ue_offline(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr)
{
    char cmd[512]={0}, imsi_str[64]={0}, ip_buf[20]={0};
    int i;

    for (i=0; i<imsi_len; i++) {
        sprintf(imsi_str+2*i, "%x", *(imsi+i)&0xF);
        sprintf(imsi_str+2*i+1, "%x", (*(imsi+i)&0xF0)>>4);
    }

    if (imsi_len == 8) {
        if (imsi_str[15] == 'f') {
            imsi_str[15] = '\0';
        }
    }

    inet_ntop(AF_INET, addr, ip_buf, 20);
    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"imsi\":\"%s\",\"ip\":\"%s\",\"enbid\":\"%d\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_UE_OFFLINE,
                       imsi_str,
                       ip_buf,
                       enb_id);
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}

void mgmt_report_ue_location_update(c_uint8_t *imsi, int imsi_len, c_uint32_t enb_id, c_uint32_t *addr)
{
    char cmd[512]={0}, imsi_str[64]={0}, ip_buf[20]={0};
    int i;

    for (i=0; i<imsi_len; i++) {
        sprintf(imsi_str+2*i, "%x", *(imsi+i)&0xF);
        sprintf(imsi_str+2*i+1, "%x", (*(imsi+i)&0xF0)>>4);
    }

    if (imsi_len == 8) {
        if (imsi_str[15] == 'f') {
            imsi_str[15] = '\0';
        }
    }

    inet_ntop(AF_INET, addr, ip_buf, 20);
    snprintf(cmd, 512, "%s/agent_c %s \'{\"type\":\"%d\",\"imsi\":\"%s\",\"ip\":\"%s\",\"enbid\":\"%d\"}\' &",
                       AGENT_C_PATH,
                       MGMT_SRV,
                       MGMT_MSG_T_UE_LOCATION_UPDATE,
                       imsi_str,
                       ip_buf,
                       enb_id);
    system(cmd);
    MGMT_PRINT("%s\n", cmd);
}
