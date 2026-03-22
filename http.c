#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT "8282"
#define BUFFER_SIZE 512
#define BACKLOG 5
/**
 Servidor TCP que recebe uma conexão, serve html e depois encerra
*/

void server(){
    char client_ip[INET6_ADDRSTRLEN]; // IP do cliente somente pra log
    void *addr; // ponteiro generico pra servir tanto como ipv4 como ipv6
    struct sockaddr_storage client_addr; // struct que serve pra ipv4 e ipv6
    socklen_t client_addr_size = sizeof client_addr; //inicializa com tamano do endereço do cliente
    int server_fd, client_fd, status; // file descriptos e status.
    int yes = 1; // isso aqui é uma gambiarra que precisa ser passado pra setsockopt
    struct addrinfo hints, *res; // struct filtro IP + porta e linked list pra resposta
    memset(&hints, 0, sizeof hints); // limpa a memoria da struct pra não conter lixo
    hints.ai_family = AF_INET; // IPV4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Vai receber o IP da maquina
    if((status = getaddrinfo(NULL,PORT,&hints, &res)) != 0){ // preenche IP e porta do servidor
        fprintf(stderr, "[ERROR] DNS LOOKUP: [%s]\n", gai_strerror(status));
        exit(1);
    }
    if((server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){ // Cria o file descriptor
        perror("[ERROR] CANNOT CREATE SOCKET");
        exit(1);
    }
    if((status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes)) == -1) { // Reuso de portas
        perror("[ERROR] CANNOT SET SOCKET OPTIONS");
        exit(1);
    }
    if((status = bind(server_fd, res->ai_addr, res->ai_addrlen)) == -1) { // bind do file descriptor com porta e IP
        perror("[ERROR] CANNOT BIND SOCKET");
        exit(1);
    }
    if((status = listen(server_fd, BACKLOG)) == -1){ //listen (pode receber conexões)
        perror("[ERROR] CANNOT LISTEN");
        exit(1);
    }
    if((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size)) ==  -1){ //aceita uma conexão
        perror("[ERROR] CANNOT ACCEPT CLIENT CONNECTION");
        exit(1);
    }
    if(client_addr.ss_family == AF_INET){ // se for ipv4 atribui a variavel addr
        struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
        addr = &(s->sin_addr);
    } else { // se for ipv6 atribui a variável addr
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_addr;
        addr = &(s->sin6_addr);
    }
    inet_ntop(client_addr.ss_family, addr, client_ip, sizeof client_ip);// ntop (network to printable) traduz um endereço de 32 ou 128 para string
    printf("CLIENT CONNECTED WITH IP %s\n", client_ip);
    char *message = "<html><head><title>Hello C</title></head><body><h1>[TCP SOCKET (C)] Hello, world!</h1></body></html>";
    char response[BUFFER_SIZE];
    //formata a string fazendo concateção e colocando na string response
    snprintf(response, sizeof response, "HTTP/1.1 200 OK\r\n" "Content-Type: text/html\r\n" "Content-Length:%zu\r\n" "\r\n" "%s", strlen(message), message);
    //envia o html para o cliente
    send(client_fd, response, strlen(response), 0);
    close(client_fd); //fecha o filedescriptor do socket do cliente
    close(server_fd); //fecha o filedescriptor do socket do servidor
    freeaddrinfo(res); //da um free na estrutura que foi alocada pelo getaddrinfo internamente
}


int main(void){
    server();
    return 0;
}