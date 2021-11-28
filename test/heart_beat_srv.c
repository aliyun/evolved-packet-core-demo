#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

// 10s
#define HEART_BEAT_MAX_LOST_TIME (10*1000*1000)

time_t time_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char **argv)
{
    char buff[512];
    struct sockaddr_in clientAddr;
    int n;
    struct sockaddr_in addr;
    int sock;
    int len = sizeof(clientAddr);
    struct timeval tv;
    time_t prev_mme_tm, prev_spgw_tm, prev_pcrf_tm, prev_hss_tm, now_tm;
    char cmd[512] = {0};

    prev_mme_tm = time_now();
    prev_spgw_tm = prev_mme_tm;
    prev_pcrf_tm = prev_mme_tm;
    prev_hss_tm = prev_mme_tm;

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }


    addr.sin_family = AF_INET;
    addr.sin_port = htons(9998);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sock);
        exit(1);
    }

    tv.tv_sec = 11;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("set timeout failed!");
        close(sock);
        exit(1);
    }

    while (1)
    {
        strcpy(buff, "  ");
        n = recvfrom(sock, buff, 511, 0, (struct sockaddr*)&clientAddr, &len);
        now_tm = time_now();
        if (n<0)
        {
            if (errno != EAGAIN)
            {
                break;
            }

        } else if (n>0) {
            buff[n] = 0;
            printf("recv=> %s\n", buff);
            if (strcmp(buff, "mme alive") == 0)
            {
                prev_mme_tm = now_tm;
            }
            else if (strcmp(buff, "spgw alive") == 0)
            {
                prev_spgw_tm = now_tm;
            }
            else if (strcmp(buff, "pcrf alive") == 0)
            {
                prev_pcrf_tm = now_tm;
            }
            else if (strcmp(buff, "hss alive") == 0)
            {
                prev_hss_tm = now_tm;
            }
        }

        if (now_tm - prev_mme_tm > HEART_BEAT_MAX_LOST_TIME) 
        {
            printf("mme lost\n");
            sprintf(cmd, "/epc/cfg/vnf_lost_reboot.sh mme &");
            system(cmd);
        }
        else if (now_tm - prev_spgw_tm > HEART_BEAT_MAX_LOST_TIME) 
        {
            printf("spgw lost\n");
            sprintf(cmd, "/epc/cfg/vnf_lost_reboot.sh spgw &");
            system(cmd);
        }
        else if (now_tm - prev_pcrf_tm > HEART_BEAT_MAX_LOST_TIME)
        {
            printf("pcrf lost\n");
            sprintf(cmd, "/epc/cfg/vnf_lost_reboot.sh pcrf &");
            system(cmd);
        }
        else if (now_tm - prev_hss_tm > HEART_BEAT_MAX_LOST_TIME)
        {
            printf("hss lost\n");
            sprintf(cmd, "/epc/cfg/vnf_lost_reboot.sh hss &");
            system(cmd);
        }
    }
    close(sock);
    return 0;
}
