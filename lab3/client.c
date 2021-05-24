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
    struct sockaddr_in server_addr;
    
    /* Create and initialize socket */
    printf("Initializing client.\n");
    sock = makeSocket(PORT_NUM, &server_addr);
    
    /* Connect to address */
    if(connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    /* Main program loop */
    printf("Initialized, sending messages.\n");
 
    while(1)
    {
        char message[] = {"HELLO"};
                
        sendto(sock, message, strlen(message), 0,
               (const struct sockaddr *) &server_addr, sizeof(server_addr));
    }
    
    return 0;
}
