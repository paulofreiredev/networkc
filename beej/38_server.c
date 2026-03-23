#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BUFFER_SIZE 512
#define BACKLOG 10

void sigchld_handler(int s){
    (void)s; //quiet unused variable warning
    // waitpid() might override errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

//get socket addr IPV4 or IPV6:
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){
    int server_fd, client_fd;
    struct addrinfo hints, *serverinfo, *p;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char *message = "<html><head><title>Hello C</title></head><body><h1>Multi process Http server</h1></body></html>";
    char response[BUFFER_SIZE];
    snprintf(response, sizeof response, "HTTP/1.1 200 OK\r\n" "Content-Type: text/html\r\n" "Content-Length: %zu\r\n" "\r\n" "%s", strlen(message), message);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my ip
    if((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    //loop through all the results and bind the first we can
    for(p = serverinfo; p != NULL; p = p->ai_next){
        if((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==  -1){
            perror("server:socket");
            continue;
        }
        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ){
            perror("setsockopt");
            exit(1);
        }
        if(bind(server_fd, p->ai_addr, p->ai_addrlen) == -1 ){
            close(server_fd);
            perror("Server:bind");
            continue;
        }
        break;
    }
    freeaddrinfo(serverinfo); //all done with this structure
    if(p == NULL){
        fprintf(stderr, "server failed to bind\n");
        exit(1);
    }
    if(listen(server_fd, BACKLOG) == -1 ){
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1 ){
        perror("sigaction");
        exit(1);
    }
    printf("Server: waiting for connections...\n");
    while(1){ //main accept loop
        sin_size = sizeof client_addr;
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &sin_size);
        if(client_fd == -1){
            perror("accept");
            continue;
        }
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        if(!fork()){ //this is a child process
            close(server_fd); // child doesn´t need the listener
            if(send(client_fd, response, strlen(response), 0) == -1){
                perror("send");
            }
            close(client_fd);
            exit(0);
        }
        close(client_fd); //parrent doesn´t need this
    }
    return 0;
}