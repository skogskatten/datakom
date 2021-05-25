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
	while(1)
	  {
		switch(mode)
		  {
			
		  case(CONNECT):			
			switch(state)
			  {
			  case(WANTS_TO_CONNECT):
				//send SYN
				//state = WAIT
				break;
				
			  case(WAIT):
				// receives SYN-ACK / Send ACK AND connect
				// OR timeout / state = WANTS_TO_CONNECT
				// state = connected -> mode = SENDING
				break;
				
			  }
			
		  case(SENDING):
			switch(event)
			  {
			  case(ACK):
				// if ACK expected / move window, reset timeout
				// else if ACK ! expected / resend from ACK sequence number
				break;

			  case(NO_ACK):
				// if window not full / send next packet, start timer
				// else if timeout / resend from last and including N (?)
				// else Window full / do nothing
				break;
			  }
			break;

		  case(WANTS_TO_CLOSE):
			switch(state)
			  {
			  case(CONNECTED):
				// Shut down (wait for acks?) / Send FIN
				// state = WAITING_FOR_ACK
				break;
			  case(WAITING_FOR_ACK):
				// Timeout / - -> state = TIMEOUT OR
				// ACK received / - -> state = WAITING_FOR_FIN (from server) OR
				// FIN received / send FIN-ACK -> state = WAITING_ACK_2(?*)
				/* *Lägg ev till en variabel som talar om ifall  klient har mottagit en FIN eller ej. Då kan
				  en återanvända samma WAITING_FOR_ACK-tillstånd */
				break;
			  case(WAITING_FOR_FIN):
				// Timeout / - -> state = TIMEOUT OR
				// FIN received / send FIN-ACK -> state = Timeout
				break;
			  case(TIMEOUT):
				//timeout / Close connection -> state = disconnected
				break;
				
			  }
		  }
	  }
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
