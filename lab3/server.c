/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "functions.c"

int main(void)
{
    int listen_sock;
    struct sockaddr_in server_addr;
    
    /* Create and initialize socket */
    printf("Initializing server.\n");
    listen_sock = makeSocket(PORT_NUM, &server_addr);
    
    /* Assign address to socket */
    if(bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    /* Main program loop */
    printf("Initialized, waiting for connections.\n");
    while(1)
    {
        int len, nOfBytes;
        char buffer[MAX_MSG_LEN];
        struct sockaddr_in client_addr;
        
        len = sizeof(client_addr);
        nOfBytes = recvfrom(listen_sock, (char *)buffer, MAX_MSG_LEN, MSG_WAITALL, (struct sockaddr*) &client_addr,
                    &len);
        
        if(nOfBytes > 0)
            printf("Client: %s\n", buffer);
    }
    
    return 0;
}
