#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define  PORT 9999 
 
int main(int argc, char *argv[])
{
    int  sockfd, num;
    struct hostent *he;
    struct sockaddr_in server;
    char *msg=NULL;
    int len=0;

    if ( argc != 3 ) {
        printf("Usage:%s [IP Address] [message]\n", argv[0]);
        exit(1);
    }

    msg = argv[2];

    if (msg == NULL) {
        printf("cmd error!\nUsage:%s [IP Address] [message]\n", argv[0]);
        exit(1);
    }

    len = strlen(msg);

    if((he=gethostbyname(argv[1]))==NULL){
        printf("gethostbyname()error\n");
        exit(1);
    }

    if((sockfd=socket(AF_INET, SOCK_STREAM, 0))==-1){
        printf("socket() error\n");
        exit(1);
    }



    bzero(&server,sizeof(server));
    server.sin_family= AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr =*((struct in_addr *)he->h_addr);
    if (connect(sockfd,(struct sockaddr *)&server,sizeof(server))==-1){
        printf("connect() err: %s\n", strerror(errno));
        exit(1);
    }

    num = send(sockfd, msg, len, 0);
    if (num != len) {
        printf("send() error! err=%s\n", strerror(errno));
        exit(1);
    } else {
        //printf("send() success!\n");
    }

    close(sockfd);

    return 0;
}
