#include "functions.h"

void ConnectionReceiver(int *state, int *mode, int *clientSock, int serverSock, fd_set *read_fd, int *lastSeqReceived, int *windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {
  
  int retval;
  fd_set active_fd;
  rtp packageReceived, packageToSend;
  
  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(localAddr->sin_addr), strlen(inet_ntoa(localAddr->sin_addr)));
  
  struct timeval read_timeout;
  
  
  printf("MODE: MODE_AWAIT_CONNECT.\n"); // Antar alla inkommande meddelanden är syn
  
  if(*state == RESET)
    *state = STATE_LISTEN;
  
  switch(*state) {
    
  default :
    printf("Undefined case: unconnected state . Exiting.\n");
    exit(EXIT_FAILURE);
      
      
  case STATE_LISTEN : // LISTENING
    // Lyssna efter ack
    
    printf("STATE: listening for syn.\n");
    
    while (*state == STATE_LISTEN) {

      retval = recv_rtp(serverSock, &packageReceived, remoteAddr);
      
      if(retval < 0) {
	printf("retval = %d. Server, listening for connection: received incorrect checksum.\n", retval);
	//	return;
      }
      else if (retval) {
	printf("Server, listening for connection: received %d bytes from %s.\n", retval, inet_ntoa(remoteAddr->sin_addr));

	// RECEIVES SYN -> SEND SYN-ACK
	if (packageReceived.flags == FLAG_SYN) {
	  
	  printf("Received SYN.\n");
	  printf("Client window size: %d\n", packageReceived.windowsize);
	  
	  *windowSize = packageReceived.windowsize;
	
	  *clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	  if(*clientSock < 0) {
	    perror("Server, creating client socket");
	    exit(EXIT_FAILURE);
	  }
	  
	  packageToSend.flags = FLAG_SYN_ACK;
	  packageToSend.windowsize = *windowSize;
	  
	  send_rtp(*clientSock, &packageToSend, remoteAddr);
	  
	  *state = STATE_AWAIT_ACK;
	}
	else
	  printf("Package not a SYN. Discarded.\n");
      }
    }


    // Await SYN-ACK-ACK -> Connect
  case STATE_AWAIT_ACK :
    
    printf("STATE: waiting for syn-ack-ack.\n");

    // active fd is the temporary file descriptor.
    FD_ZERO(&active_fd);
    FD_SET(*clientSock, &active_fd);
      
    switch(select(*clientSock + 1, &active_fd, NULL, NULL, NULL)) {
    case -1:
      perror("Server, select c_sockfd");
      exit(EXIT_FAILURE);
    case 0:
      printf("Server, select c_sockfd: nothing to read. Returning to listen state.\n");
      close(*clientSock);
      *state = STATE_LISTEN;
      FD_ZERO(&active_fd);
      break;
    default:
      retval = recv_rtp(c_sockfd, &packageReceived, &c_addr2);
      if(retval < 0) {
	printf("Server, listening for syn-ack-ack: incorrect checksum.");
      }
      //	else {
      printf("Server, listening for syn-ack-ack: received %d bytes from %s.\n", retval, inet_ntoa(c_addr2.sin_addr));
      printf("Msg: %s\n", packageReceived.data);
      memset(&packageReceived.data, '\0', MAX_DATA_LEN);


      retval = recv_rtp(c_sockfd, &packageReceived, &c_addr2);
      if (retval < 0) {
	perror("Something: incorrect checksum received.");
      }
      printf("Msg flag: %d", packageReceived.flags);
      printf("Msg data: %s", packageReceived.data);
      state = STATE_CONNECTED;
	
      //	}
      
    case STATE_CONNECTED :
      printf("STATE: syn-ack-ack received.\n");
      *mode = MODE_CONNECTED;
      *state = RESET;
     
    }
    if(*mode != MODE_CONNECTED)
      break;


  }
