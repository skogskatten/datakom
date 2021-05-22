/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, arm                             *
 * Purpose:                                      *
 * ***********************************************/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "functions.c" //contains shared funcs betwen server/client

#define MAXMSG 512
#define PORT   5555

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

int main(void)
{
    int listen_sock;

    /* Create and initialize socket */
    printf("Initializing server.\n");
    listen_sock = makeSocket(PORT);
    
    /* Main program loop */
    printf("Initialized, waiting for connections.\n");
    while(1)
    {
        char buffer[MAXMSG];
        int len;
        struct sockaddr_in server_addr, client_addr;

        len = sizeof(client_addr);
        
        recvfrom(listen_sock, (char *)buffer, MAXMSG, MSG_WAITALL,
                (struct sockaddr *) &client_addr, &len);
        
        printf("Client: %s\n", buffer);
    }
    
    return 0;
}






