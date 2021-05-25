/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "../lab3/functions.c"

int main(void)
{
    int sock;
    struct sockaddr_in server_addr;
    
    /* Create and initialize socket */
    printf("Initializing server.\n");
    sock = makeSocket(PORT_NUM, &server_addr);
    
    /* Assign address to socket */
    if(bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    /* Main program loop */
    printf("Initialized, waiting for data.\n");
    
    rtp package;
    print_rtp_header();
    
    while(1)
    {
        if(recv_rtp(sock, &package, &server_addr) >= 0)
            printf("[RECV] ");
        else
            printf("[TRSH] ");
        print_rtp(&package);
    }
    return 0;
}
