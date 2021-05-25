/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "functions.c"

int main(void)
{
  int sock, state, i, j;
  struct sockaddr_in server_addr;
  fd_set fd_set, read_fd_set, write_fd_set;
    
  /* Create and initialize socket */
  printf("Initializing server.\n");
  sock = makeSocket(PORT_NUM, &server_addr);
    
  /* Assign address to socket */
  if(bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
	  perror("bind");
	  exit(EXIT_FAILURE);
    }

  /* Initialize fd_set to a null (empty) set */
  FD_ZERO(&fd_set);
  /* Add sock to the fd_set */
  FD_SET(sock, &fd_set);


  int mode, state, event;
  /* Main program loop */
  printf("Initialized, waiting for connections.\n");
  while(1)
    {
	  read_fd_set = fd_set;
	  write_fd_set = fd_set;
    
	  /* Select selects the sockets ready for input or output. Not clear if we need write_fd_set here, or at all */
	  if(select(FD_SETSIZE, &read_fd_set, &write_fd_set, NULL, NULL, NULL) < 0) {
		perror("Select failed\n");
		exit(EXIT_FAILURE);
	  }
		
	  /*nedan följer pseudokod med förslag på tillståndsflöde. Fungerar endast för 1 klient? Kanske? Varje ny klient får, efter connect, en egen socket där meddelanden från klienten tas emot/skickas(?), vilket gör AWAIT_CONNECT inaktuellt för de andra socketarna, samtidig som socket 0, serverns "publika" socket aldrig lämnar uppkoppling. Borde funka bra med en if-sats som slänger ut alla klientbrevlådor ur AWAIT_CONNECT*/
	  for(i = 0; i < FD_SETSIZE; i++)
		{
		if(FD_ISSET(i, read_fd_set))
		  {
		  switch(mode)
			{
			  
			case(AWAIT_CONNECT):
			  //Only sock should ever enter this state
			  if(curr != sock){
				mode = CONNECTED;
			  break;
			  }
			  // Listening
			  // receives SYN / send SYN-ACK
			  // state Wait
			  // Receives ACK -> Connected OR timeout -> Listening
			  break;
			
			case(CONNECTED):
			  switch(state)
				{
			  case(RECIEVE): //wait
				switch(event)
				  {
				case(PACKAGE_EXPECTED):
				  //send ack
				  break;
				case(PACKAGE_WRONG):
				  //send prev ack
				  break;
				case(PACKAGE_FIN):
				  //switch mode teardown
				  break;
				}
			  }
			  break;
			  
			case(TEARDOWN): /* Server has received a FIN */
			  // Send FIN-ACK
			  // Shutting down, preparing to close connection
			  // Send FIN
			  // Waiting for FIN-ACK
			  // Timeout -> disconnect OR ACK received -> disconnect
			  break;
			  
			default;
			//no op
			}//switch(mode)
		}//if FD_ISSET
	  }//for FD_SETSIZE
    }//while
    
  //  unlink((struct sockaddr *) &server_addr);
  return 0;
}
