/*************************************************
 * File: client.c                                *
 * Name: Client program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose: lab3                                 *
 * ***********************************************/

#include "../lab3/functions.c"

int main(void)
{
    int sock;
    time_t t;
    struct sockaddr_in server_addr;
    
    /* Init rand */
    srand((unsigned) time(&t));
    
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
    
    rtp package;
    package.flags = 0;
    package.id = sock;
    package.seq = 0;
    package.windowsize = 16;
    memcpy(package.data, "Hello world!\0", sizeof("Hello world!\0"));
    
    /* Main program loop */
    printf("Initialized, sending messages.\n");
    print_rtp_header();
    
    while(1)
    {
        send_rtp(sock, &package, NULL);
        
        printf("[SENT] ");
        print_rtp(&package);
        
        package.seq += 1;
        usleep(1000000);
    }
    
    return 0;
}
