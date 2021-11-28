#define _GNU_SOURCE
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define VLEN 100
#define BUFSIZE 1500
#define TIMEOUT 1

int main(int argc, char *argv[])
{
    int sockfd, retval, i;
    struct sockaddr_in addr;
    struct mmsghdr msgs[VLEN];
    struct iovec iovecs[VLEN];
    char bufs[VLEN][BUFSIZE+1];
    struct timespec timeout;
    unsigned int pkt_cnt = 0;
    unsigned long bytes = 0;
    struct timeval start, end;
    unsigned int delta_ms = 1;
    char *ip = "30.12.22.0";
    unsigned short port = 61001;
    size_t size = 1024*1500;

    if (argc == 3) {
        ip = argv[1];
        port = atoi(argv[2]);
    } else {
        printf("Usage: udp_server [ip] [port]\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size_t)) < 0) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }

    memset(msgs, 0, sizeof(msgs));
    for (i = 0; i < VLEN; i++) {
        iovecs[i].iov_base         = bufs[i];
        iovecs[i].iov_len          = BUFSIZE;
        msgs[i].msg_hdr.msg_iov    = &iovecs[i];
        msgs[i].msg_hdr.msg_iovlen = 1;
    }

    while(1) {
        timeout.tv_sec = TIMEOUT;
        timeout.tv_nsec = 0;

        if (pkt_cnt == 0) {
            retval = recvmmsg(sockfd, msgs, 1, 0, &timeout);
        } else {
            retval = recvmmsg(sockfd, msgs, VLEN, 0, &timeout);
        }

        if (retval == -1) {
            perror("recvmmsg()");
            exit(EXIT_FAILURE);
        }

        if (pkt_cnt == 0) {
            gettimeofday( &start,NULL );
            pkt_cnt++;
            continue;
        }

        for (i = 0; i < retval; i++) {
            bytes += msgs[i].msg_len;
            pkt_cnt++;

            if (pkt_cnt%1000 == 0 || pkt_cnt > 200000) {
                gettimeofday(&end, NULL);
                delta_ms = 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
                printf("Rcv: %d pkts,\t%dBytes,\t%dkbps\n", pkt_cnt, bytes, (bytes*8)/delta_ms);
            }
        }
    }
    exit(EXIT_SUCCESS);
}
