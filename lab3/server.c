/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "functions.h"

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






