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
    /* "If the socket sockfd is of type SOCK_DGRAM, then addr is the
     * address to which datagrams are sent by default, and the only
     * address from which datagrams are received.
     */
    if(connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    /* Main program loop */
    printf("Initialized, sending messages.\n");
 
    char message[] = {"HELLO"};
                
    sendto(sock, message, strlen(message), 0,
        (const struct sockaddr *) NULL, sizeof(server_addr)); //NULL added in address field, test this
    
    printf("sent message, recieving next.. \n");
    
    int nOfBytes;
        //unsigned int len;
        char buffer[MAX_MSG_LEN];
        //struct sockaddr_in client_addr;
        
        //len = sizeof(server_addr);
        nOfBytes = recvfrom(sock, (char *)buffer, MAX_MSG_LEN, 0, NULL, NULL);
        if(nOfBytes > 0)
            printf("Server: %s\n", buffer);
    
    return 0;
}
