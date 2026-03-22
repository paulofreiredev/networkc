#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

/*
    Usa a função getaddrinfo para fazer a tradução de um dominio válido como google.com em endereços IPv4 e IPv6
    Uso: ./bin/dns google.com (pode substituir por qualquer site válido)
*/

int main(int argc, char *argv[]){
    int status; //manipular o status do dns lookup
    struct addrinfo hints; //sruct que contem IP e porta para usada como filtro
    struct addrinfo *res; // linkedlist com os endereços retornados
    char ipstr[INET6_ADDRSTRLEN]; // string de tamanho do IPV6 que cabe tanto ipv4 como ipv6

    memset(&hints, 0, sizeof hints); // Pra evitar lixo na memória cause um comportamento inesperado no programa
    hints.ai_family = AF_UNSPEC; // ipv4 ou ipv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // Flag que indica que não vou passar um endereço IP

    if((status = getaddrinfo(argv[1], NULL, &hints, &res) != 0)){
        fprintf(stderr, "Error: %s\n", gai_strerror(status)); // gai strerror significa get address info string error | faz a tradução
        exit(1);
    }
    // aqui interamos sobre a lista linkada para exibir os endereços IP
    struct addrinfo *current; 
    for(current = res; current != NULL; current = current->ai_next){
        void *addr; // ponteiro generico pra servir tanto como ipv4 como ipv6
        char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;
        if(current->ai_family == AF_INET){ // se for ipv4
            ipv4 = (struct sockaddr_in *) current->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // se for ipv6
            ipv6 = (struct sockaddr_in6 *) current->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(current->ai_family, addr, ipstr, sizeof ipstr); // ntop (network to printable) traduz um endereço de 32bits para string
        printf("%s:%s\n", ipver, ipstr);
    }
    freeaddrinfo(res); //da um free na estrutura que foi alocada pelo getaddrinfo internamente
    return 0;
}