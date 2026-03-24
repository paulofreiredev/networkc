#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"
#define MAXDATASIZE 512

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    char buffer[MAXDATASIZE];
    struct addrinfo hints, *serverinfo, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if(argc != 2){
        fprintf(stderr, "Usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(argv[1], PORT, &hints, &serverinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    //loop for all results and then connect the first we can
    for(p = serverinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), ipstr, sizeof ipstr);
        printf("client attempt to connect to %s\n", ipstr);
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), ipstr, sizeof ipstr);
    printf("client connected to %s\n", ipstr);
    freeaddrinfo(serverinfo);
    if((numbytes = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1){
        perror("recv");
        exit(1);
    }
    buffer[numbytes] = '\0';
    printf("client received: %s\n", buffer);
    close(sockfd);
    return 0;
}