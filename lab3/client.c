/*************************************************
 * File: client.c                                *
 * Name: Client program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose: lab3                                 *
 * ***********************************************/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "functions.c" //contains shared funcs betwen server/client

#define HOSTNAME_MAX_LENGTH 50
#define MESSAGE_MAX_LENGTH 512
#define PORT_NUMBER   5555

int makeSocket(u_int16_t port)
{
    int sock, value_one = 1;
    struct sockaddr_in name;

    /* Create socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Give socket a name, port nr and address. */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Set sock opt */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
    (void *)&value_one, sizeof(value_one)) < 0)
    {
        perror("filter");
        exit(EXIT_FAILURE);

    }
    
    /* Assign address to socket */
    if(bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void initSocketAddress(struct sockaddr_in *name, char *host_name,
        unsigned short int port)
{
    struct hostent *host_info;
    
    name->sin_family = AF_INET;
    name->sin_port = htons(port);
    host_info = gethostbyname(host_name);
    if(host_info == NULL)
    {
        fprintf(stderr, "initSocketAddress - Unknown host %s\n", host_name);
        exit(EXIT_FAILURE);
    }
    name->sin_addr = *(struct in_addr *)host_info->h_addr;
}

int main(void)
{
    int listen_sock;
    struct sockaddr_in server_addr;

    /* Create and initialize socket */
    printf("Initializing client.\n");
    listen_sock = makeSocket(PORT_NUMBER);
    
    initSocketAddress(&server_addr, "lapkat2", PORT_NUMBER);

    /* Main program loop */
    printf("Initialized, sending message. GLHF.\n");
    while(1)
    {
        char buffer[MESSAGE_MAX_LENGTH];
        int len, n;
        struct sockaddr_in client_addr;
        
        sendto(listen_sock, (const char *)"hello", strlen("hello"), MSG_CONFIRM,
                (const struct sockaddr *) &server_addr, sizeof(server_addr));
        printf("msg sent");

        close(listen_sock);
    }
    
    return 0;
}






