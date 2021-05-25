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
				//send SYN / - -> state = WAIT
				break;
				
			  case(WAIT):
				switch(event)
				  {					
				  case(SYN-ACK):
					// receives SYN-ACK / Send ACK AND connect -> mode = SENDING
					break;
				  case(TIMEOUT):
					// timeout / - -> state = WANTS_TO_CONNECT					
					break;
				  }
				break;
			  }
			
		  case(SENDING):
			switch(event)
			  {
			  case(ACK): /* Skall vi kontrollera om korrekt N här eller innan switch? */
				// if ACK expected / move window, reset timeout
				// else if ACK ! expected / resend from ACK sequence number
				break;
				
			  case(NO_ACK): /* If-satser eller ytterliggare switch-case? Enklare med några if i det här fallet? */
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
				// Shut down (wait for acks?) / Send FIN -> state = WAITING_FOR_ACK
				break;
					
			  case(WAITING_FOR_ACK):
				switch(event)
				  {
				  case(timeout):
					// Timeout / - -> state = TIMEOUT OR
					break;
						
				  case(ACK):
					// ACK received / - -> state = WAITING_FOR_FIN (from server) OR
					break;

				  case(FIN):
					// FIN received / send FIN-ACK -> state = WAITING_ACK_2(?*)
					break;
					/* *Lägg till en variabel som talar om ifall  klient har mottagit en FIN eller ej. Då kan
					   en återanvända samma WAITING_FOR_ACK-tillstånd */
				  }
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
	
    return 0;
}
