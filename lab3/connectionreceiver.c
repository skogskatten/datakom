#include "functions.h"

void ConnectionReceiver(int *state, int *mode, int *clientSock, int serverSock, fd_set *read_fd, fd_set *write_fd, int *lastSeqReceived, int *lastSeqSent, int *windowSize, rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {
  
  int retval;
  fd_set active_fd;
  rtp packageReceived, packageToSend;
  
  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(localAddr->sin_addr), strlen(inet_ntoa(localAddr->sin_addr)));
  
  struct timeval read_timeout;
  
  
  printf("MODE: MODE_AWAIT_CONNECT.\n"); 
  
  if(*state == RESET)
    *state = STATE_LISTEN;
  
  while(*mode == MODE_AWAIT_CONNECT) {
    switch(*state) {
    
    default :
      printf("Undefined case: unconnected state . Exiting.\n");
      exit(EXIT_FAILURE);
      
      
    case STATE_LISTEN : // LISTENING FOR SYN
        
      printf("STATE_LISTEN: listening for syn.\n");
    
      while (*state == STATE_LISTEN) {
	retval = recv_rtp(serverSock, &packageReceived, remoteAddr);
	if(retval < 0) {
	  printf("retval = %d. Server, listening for connection: received incorrect checksum.\n", retval);
	}
	else if (retval) {
	  printf("Server, listening for SYN: received %d bytes from %s.\n", retval, inet_ntoa(remoteAddr->sin_addr));

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
	    *lastSeqSent = 1;
	    memcpy(&packageToSend.seq, lastSeqSent, sizeof(*lastSeqSent));
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
    
      printf("STATE_AWAIT_ACK: waiting for syn-ack-ack.\n");

      // active fd is the temporary file descriptor.
      FD_ZERO(&active_fd);
      FD_SET(*clientSock, &active_fd);
    
      read_timeout.tv_sec = SHORT_TIMEOUT;
      read_timeout.tv_usec = 0;
    
      while(*state == STATE_AWAIT_ACK) {
	switch(select(*clientSock + 1, &active_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  perror("STATE_AWAIT_ACK");
	  exit(EXIT_FAILURE);
	case 0:
	  printf("STATE_AWAIT_ACK: Timeout. Returning to listen state.\n");
	  close(*clientSock);
	  FD_ZERO(&active_fd);
	  *state = STATE_LISTEN;
	  break;
	default:
	  retval = recv_rtp(*clientSock, &packageReceived, remoteAddr);
	  if(retval < 0) {
	    printf("STATE_AWAIT_ACK: listening for syn-ack-ack: incorrect checksum.");
	    *state = STATE_LISTEN;
	    close(*clientSock);
	  }
	  else {
	    printf("STATE_AWAIT_ACK: listening for syn-ack-ack: received %d bytes from %s.\n", retval, inet_ntoa(remoteAddr->sin_addr));

	    if (packageReceived.flags != FLAG_ACK || packageReceived.seq != (unsigned) *lastSeqSent) {
	      printf("STATE_AWAIT_ACK: Package received not an ack or had incorrect seq.num.. Discarded.\n");
	    }
	    else {
	      printf("STATE_AWAIT_ACK: ACK received, connecting.");
	      *lastSeqReceived = packageReceived.seq;
	      *state = STATE_CONNECTED;
	    }
	  }
	}
      }
      if(*state != STATE_CONNECTED)
	break;
      
    case STATE_CONNECTED :
      printf("STATE_CONNECTED: syn-ack-ack received. Going to MODE_CONNECTED.\n");
      *mode = MODE_CONNECTED;
      *state = RESET;
      sendWindow = AllocateWindow(*windowSize);
      FD_SET(*clientSock, read_fd);
      FD_SET(*clientSock, write_fd);
      return;
    }
  }
}
