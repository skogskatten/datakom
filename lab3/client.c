/*************************************************
 * File: client.c                                *
 * Name: Client program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose: lab3                                 *
 * ***********************************************/

#include "functions.c"

int main(void)
{
    int sock; 
    
    /* Create and initialize socket */
    printf("Initializing client.\n");
    sock = makeSocket(PORT_NUM);
    
    /* Main program loop */
    printf("Initialized, waiting for connections.\n");
 
        int len, nOfBytes;
        char message[] = {"HELLO"};
        struct sockaddr_in server_addr;
        
        sendto(sock, message, strlen(message), MSG_CONFIRM,
               (const struct sockaddr *) &server_addr, sizeof(server_addr));
    
    
    return 0;
}





