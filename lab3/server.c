/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "functions.c"

int main(void)
{
  int sock, state;
  struct sockaddr_in server_addr;
  fd_set sock_set, read_set, write_set;
    
  /* Create and initialize socket */
  printf("Initializing server.\n");
  sock = makeSocket(PORT_NUM, &server_addr);
    
  /* Assign address to socket */
  if(bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
      perror("bind");
      exit(EXIT_FAILURE);
    }

  /* Initialize sock_set to a null (empty) set */
  FD_ZERO(&sock_set);
  /* Add sock to the sock_set */
  FD_SET(sock, &sock_set);
    
  /* Main program loop */
  printf("Initialized, waiting for connections.\n");
  while(1) {
      
    read_set = sock_set;
      
    write_set = sock_set;
    
    /* Något händer, avgörs med select */

    /*nedan följer pseudokod med förslag på tillståndsflöde. */
    switch(state) {
      
    case(INIT):/* INIT: Initial state, receives packets */
      receive;
      if(order)
	switch(FLAG) {
	case(SYN):
	  break;
      	case(FIN):
	  break;
	case(ACK):
	  break;
	case(DATA):
	  break;
	}
      else
	/* resend last ack */;
      break;
      
    case(WAIT_ACK):
      break;

    case(WAIT_SYN-ACK):
      break;

    } //switch end
    
  } //main while end

  //  unlink((struct sockaddr *) &server_addr);
  return 0;
}
