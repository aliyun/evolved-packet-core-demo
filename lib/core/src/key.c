#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "md5.h"
#include "core_debug.h"


#define INTF_P_MAC_FILE "/tmp/pmac"

static void get_cpuid(char *PSN)
{
    int varEAX, varEBX, varECX, varEDX;
    char str[9], psn_tmp[30] = {0};
    //%eax=1 gives most significant 32 bits in eax 
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (1));
    sprintf(str, "%08X", varEAX); //i.e. XXXX-XXXX-xxxx-xxxx-xxxx-xxxx
    sprintf(PSN, "%C%C%C%C-%C%C%C%C", str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    //%eax=3 gives least significant 64 bits in edx and ecx [if PN is enabled]
    __asm__ __volatile__ ("cpuid"   : "=a" (varEAX), "=b" (varEBX), "=c" (varECX), "=d" (varEDX) : "a" (3));
    sprintf(str, "%08X", varEDX); //i.e. xxxx-xxxx-XXXX-XXXX-xxxx-xxxx
    sprintf(psn_tmp, "%s-%C%C%C%C-%C%C%C%C", PSN, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
    sprintf(str, "%08X", varECX); //i.e. xxxx-xxxx-xxxx-xxxx-XXXX-XXXX
    sprintf(PSN, "%s-%C%C%C%C-%C%C%C%C", psn_tmp, str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7]);
}

static int get_two_char(char* ori, char* out)
{
    char c;

    if (!ori || !out) {
        return -1;
    }

    c = *ori;
    if (c >= '0' && c <= '9') {
        *out = c;
    } else if (c >= 'A' && c <= 'F') {
        *out = c;
    } else if (c >= 'a' && c <= 'f') {
        *out = c - 32;
    } else {
        return -1;
    }

    c = *(ori+1);
    if (c >= '0' && c <= '9') {
        *(out+1) = c;
    } else if (c >= 'A' && c <= 'F') {
        *(out+1) = c;
    } else if (c >= 'a' && c <= 'f') {
        *(out+1) = c - 32;
    } else {
        return -1;
    }

    return 0;
}

static int parse_mac_addr(char *ori, char* out)
{
    if (!ori || !out) {
        return -1;
    }

    if (strlen(ori) < 17) {
        return -1;
    }

    if (*(ori+2) != ':' ||*(ori+5) != ':' ||*(ori+8) != ':' ||*(ori+11) != ':' ||*(ori+14) != ':') {
        return -1;
    }

    if (get_two_char(ori, out) != 0) {
        return -1;
    }
    if (get_two_char(ori+3, out+2) != 0) {
        return -1;
    }
    if (get_two_char(ori+6, out+4) != 0) {
        return -1;
    }
    if (get_two_char(ori+9, out+6) != 0) {
        return -1;
    }
    if (get_two_char(ori+12, out+8) != 0) {
        return -1;
    }
    if (get_two_char(ori+15, out+10) != 0) {
        return -1;
    }

    *(out+12) = '\0';

    return 0;
}

static int get_mac_addr(char *intf, char *mac)
{
    char buf[256];
    FILE* fp = NULL;
    int len1, len2;

    if (!intf || !mac) {
        return -1;
    }

    sprintf(buf, "ethtool -P %s > %s 2>&1", intf, INTF_P_MAC_FILE);
    system(buf);

    fp = fopen(INTF_P_MAC_FILE, "r");
    if (!fp) {
        return -1;
    }

    if (fgets(buf, 255, fp) == NULL) {
        goto err;
    }

    len1 = strlen("Permanent address: ");
    len2 = strlen(buf);

    if (len2 < len1 + 17) {
        goto err;
    }

    if (strncmp(buf, "Permanent address: ", len1) != 0) {
        goto err;
    }

    if (parse_mac_addr(&buf[len1], mac) != 0) {
        goto err;
    }

    fclose(fp);
    return 0;

err:
    fclose(fp);
    return -1;
}

int get_sys_serial_num(char *s1_intf, char *serial)
{
    char cpu_info[30];
    char mac[16] = {0};

    if (!s1_intf || !serial) {
        return -1;
    }

    get_cpuid(cpu_info);
    if (get_mac_addr(s1_intf, mac) != 0) {
        d_error("get s1 intf info failed %s", s1_intf);
        return -1;
    }

    serial[0] = cpu_info[0];
    serial[1] = cpu_info[1];
    serial[2] = cpu_info[2];
    serial[3] = cpu_info[3];
    serial[4] = cpu_info[5];
    serial[5] = cpu_info[6];
    serial[6] = cpu_info[7];
    serial[7] = cpu_info[8];
    serial[8] = mac[0];
    serial[9] = mac[1];
    serial[10] = mac[2];
    serial[11] = mac[3];
    serial[12] = mac[4];
    serial[13] = mac[5];
    serial[14] = mac[6];
    serial[15] = mac[7];
    serial[16] = mac[8];
    serial[17] = mac[9];
    serial[18] = mac[10];
    serial[19] = mac[11];
    serial[20] = '\0';
    return 0;
}

int check_soft_serial(char *s1_intf, char *serial)
{
    char sys_serial[32] = {0};

    if (serial == NULL) {
        return 0;
    }

    if (get_sys_serial_num(s1_intf, sys_serial) != 0) {
        d_error("get_sys_serial_num failed!");
        return 0;
    }

    if (strcmp(serial, sys_serial) == 0) {
        return 1;
    }
    d_error("sys_serial is %s", sys_serial);
    
    return 0;
}

int check_soft_serial_and_key(char *s1_intf, char *serial, char *key) 
{
    unsigned char real_serial[32] = {0};
    char real_key[64] = {0};

    if (serial == NULL || key == NULL) {
        return 0;
    }

    if (check_soft_serial(s1_intf, serial) != 1) {
        return 0;
    }

    real_serial[0] = serial[0];
    real_serial[1] = serial[1];
    real_serial[2] = serial[3];
    real_serial[3] = serial[2];
    real_serial[4] = serial[4];
    real_serial[5] = serial[5];
    real_serial[6] = serial[6];
    real_serial[7] = serial[7];
    real_serial[8] = serial[8];
    real_serial[9] = serial[9];
    real_serial[10] = serial[10];
    real_serial[11] = serial[11];
    real_serial[12] = serial[12];
    real_serial[13] = serial[13];
    real_serial[14] = serial[14];
    real_serial[15] = serial[15];
    real_serial[16] = serial[17];
    real_serial[17] = serial[16];
    real_serial[18] = serial[18];
    real_serial[19] = serial[19];
    real_serial[20] = '\0';

    md5_encrpt(real_serial, real_key);
    //printf("serial: %s\nreal_key: %s\n", real_serial, real_key);

    if (strcmp(key, real_key) == 0) {
        return 1;
    }

    return 0;
}
