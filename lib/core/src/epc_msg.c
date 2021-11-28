#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <errno.h>  
#include <string.h>  
#include <unistd.h>  
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "core_epc_msg.h"
#include "core_debug.h"



#define MME_SOCKET_PATH "/var/tmp/epc_mme_addr"
#define UPF_SOCKET_PATH "/var/tmp/epc_upf_addr"
#define EXT_SOCKET_PATH "/var/tmp/epc_ext_addr"

static int server_init(const char* sockAddr)
{  
    struct sockaddr_un serverAddr;
    int fd=-1, rc;

    unlink(sockAddr);

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return fd;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_LOCAL;
    strncpy(serverAddr.sun_path, sockAddr, sizeof(serverAddr.sun_path));

    rc = bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (rc != 0) {
        perror("bind error");
        return -1;
    }

    rc = listen(fd, 3);
    if ( rc != 0) {
        perror("listen error");
        return -1;
    }

    printf("srv socket %s is ready, fd=%d\n", sockAddr, fd);

    return fd;
}

static int client_init(const char* sockAddr)
{
    struct sockaddr_un serverAddr;
    struct stat statbuf;
    int fd=-1, rc;

    if ((rc= stat(sockAddr, &statbuf)) < 0) {
        printf("client_init %s not found!\n", sockAddr);
        return -1;
    }

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return fd;
    }

    if ((rc = fcntl(fd, F_SETFD, FD_CLOEXEC)) != 0) {
        printf("client_init %s FD_CLOEXEC failed!\n", sockAddr);
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sun_family = AF_LOCAL;
    strncpy(serverAddr.sun_path, sockAddr, sizeof(serverAddr.sun_path));

    rc = connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if ( rc != 0 ) {
        printf("client_init %s connect server failed! err=%s\n", sockAddr, strerror(errno));
        return -1;
    }

    //printf("client socket %s is ready, fd=%d\n", sockAddr, fd);

    return fd;
}

int epc_msg_send(int fd, const EpcMsgHeader *msg)
{
    unsigned int totalLen;
    int rc;
    int ret=0;

    totalLen = sizeof(EpcMsgHeader) + msg->dataLength;
    rc = write(fd, msg, totalLen);
    if ( rc < 0 ) {
        printf("%s %d, err=%s\n", __FUNCTION__, __LINE__, strerror(errno));
        ret = -1;
    }
    else if (rc != (unsigned int)totalLen) {
        printf("%s %d, rc=%d, totalLen=%d\n", __FUNCTION__, __LINE__, rc, totalLen);
        ret = -1;
    }

    return ret;
}

int epc_msg_send_once(EpcMsgEid dst, const EpcMsgHeader *msg)
{
    int fd = -1;
    int ret = 0;

    switch(dst) {
        case EID_UDF:
            fd = client_init(UPF_SOCKET_PATH);
            break;
        case EID_MME:
            fd = client_init(MME_SOCKET_PATH);
            break;
        case EID_EXT:
            fd = client_init(EXT_SOCKET_PATH);
            break;
        default:
            break;
    }

    if (fd < 0) {
        printf("epc_msg_send_once client_init() failed!\n");
        return -1;
    }

    ret = epc_msg_send(fd, msg);

    close(fd);
    return ret;
}

static int wait_for_data_available(int fd, unsigned int timeout)
{
   struct timeval tv;
   fd_set readFds;
   int rc;

   FD_ZERO(&readFds);
   FD_SET(fd, &readFds);

   tv.tv_sec = timeout / MSECS_IN_SEC;
   tv.tv_usec = (timeout % MSECS_IN_SEC) * USECS_IN_MSEC;

   rc = select(fd+1, &readFds, NULL, NULL, &tv);
   if ((rc == 1) && (FD_ISSET(fd, &readFds))) {
      return 0;
   } else {
      return 9008;
   }
}

int epc_msg_recv(int fd, EpcMsgHeader **msg, unsigned int *timeout)
{
   int rc;
   int ret=0;
   int totalReadSoFar=0;
   int totalRemaining=0;
   char *inBuf=NULL;

   if (msg == NULL) {
       printf("buf is NULL!");
       return 9003;
   }

   if ((*msg)->dataLength < 1024) {
       printf("msg data len is smaller than 1024!");
       return 9003;
   }
   memset(*msg, 0, sizeof(EpcMsgHeader));

   if (timeout) {
      if ((ret = wait_for_data_available(fd, *timeout)) != 0) {
         return ret;
      }
   }

   rc = read(fd, *msg, sizeof(EpcMsgHeader));
   if ((rc == 0) || ((rc == -1) && (errno == 131))) {
      /* broken connection */
      return 9002;
   } else if (rc < 0 || rc != sizeof(EpcMsgHeader)) {
      printf("bad read, fd=%d rc=%d errno=%d\n", fd, rc, errno);
      return 9002;
   }

   if ((*msg)->dataLength == 0) {
       return 0;
   }

   if ((*msg)->dataLength > 1024) {
       EpcMsgHeader *msg_new = (EpcMsgHeader *) malloc(sizeof(EpcMsgHeader) + (*msg)->dataLength);
       if (msg_new == NULL) {
           printf("malloc to %d bytes failed", (unsigned int)sizeof(EpcMsgHeader) + (*msg)->dataLength);
           return 9002;
       }
       memcpy(msg_new, *msg, sizeof(EpcMsgHeader));
       *msg = msg_new;
   }

   totalRemaining = (*msg)->dataLength;

   inBuf = (char *) (*msg + 1);
   while (totalReadSoFar < (*msg)->dataLength) {
       //printf("reading segment: soFar=%d total=%d", totalReadSoFar, totalRemaining);
       if (timeout) {
           if ((ret = wait_for_data_available(fd, *timeout)) != 0)
           {
               free(*msg);
               return ret;
           }
       }

       rc = read(fd, inBuf, totalRemaining);
       if (rc <= 0) {
           printf("bad data read, rc=%d errno=%d readSoFar=%d remaining=%d", rc, errno, totalReadSoFar, totalRemaining);
           free(*msg);
           return 9002;
       } else {
           inBuf += rc;
           totalReadSoFar += rc;
           totalRemaining -= rc;
       }
   }

   return 0;
}

int epc_msg_server_init(EpcMsgSockType type)
{
    int fd = -1;

    switch(type) {
        case EPC_SK_T_UDF:
            fd = server_init(UPF_SOCKET_PATH);
            break;
        case EPC_SK_T_MME:
            fd = server_init(MME_SOCKET_PATH);
            break;
        case EPC_SK_T_EXT:
            fd = server_init(EXT_SOCKET_PATH);
            break;

        default:
            break;
    }

    return fd;
}

int epc_msg_client_init(EpcMsgSockType type)
{
    int fd = -1;

    switch(type) {
        case EPC_SK_T_UDF:
            fd = client_init(UPF_SOCKET_PATH);
            break;
        case EPC_SK_T_MME:
            fd = client_init(MME_SOCKET_PATH);
            break;
        case EPC_SK_T_EXT:
            fd = client_init(EXT_SOCKET_PATH);
            break;

        default:
            break;
    }

    return fd;
}


/*******************************************************
 * dedicated APIs
 * ****************************************************/
int nofity_flow_info_to_ext_module(const char* cmd)
{
    char buf[sizeof(EpcMsgHeader)+1024] = {0};
    EpcMsgHeader *msg = (EpcMsgHeader *)buf;

    if (cmd == NULL) {
        return -1;
    }
    
    msg->type = MSG_T_GTP_FLOW_SYN;
    msg->dataLength = strlen(cmd) + 1;

    if (msg->dataLength > 1024) {
        return -1;
    }
    memcpy(msg+1, cmd, msg->dataLength);
    
    return epc_msg_send_once(EID_EXT, msg);
}

status_t vnf_send_heart_beat(c_int8_t *msg)
{
    struct sockaddr_in addr;
    int sock;

    d_assert(msg, return CORE_ERROR, "heart beat msg is null");
 
    if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        d_error("init socket error");
        return CORE_ERROR;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HEART_BEAT_SRV_PORT);
    addr.sin_addr.s_addr = inet_addr(HEART_BEAT_SRV_ADDR);
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        d_error("init socket error");
        close(sock);
        return CORE_ERROR;
    }
 
    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        d_error("send heart beat failed!");
    }
    close(sock);
    return CORE_OK;
}
