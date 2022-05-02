#include "functions.h"

void ConnectionSender(int *state, int *mode, int sockfd, fd_set read_fd, fd_set write_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {

  int retval;

  rtp packageToSend, packageReceived;
  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(localAddr->sin_addr), sizeof(inet_ntoa(localAddr->sin_addr)));

  struct timeval read_timeout;

  fd_set active_fd;
  
  printf("MODE_AWAIT_CONNECT.\n");
  
  while(*mode == MODE_AWAIT_CONNECT) {
    
    switch(*state) {
    default :
      printf("Undefined case: state . Exiting.\n");
      exit(EXIT_FAILURE);

      
    case STATE_WANT_CONNECT :
      printf("STATE_WANT_CONNECT: Sending syn.\n");
      
      packageToSend.flags = FLAG_SYN;
      *lastSeqSent = 1;
      packageToSend.seq = *lastSeqSent;
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, &windowSize, sizeof(windowSize));
      memcpy(&packageToSend.windowsize, &windowSize, sizeof(windowSize)); // This will inform the server of the window size we will use.
      
      send_rtp(sockfd, &packageToSend, remoteAddr);
            
      printf("Sent:\n");
      print_rtp(&packageToSend);
      
      *state = STATE_AWAIT_SYN_ACK;
      
      
    case STATE_AWAIT_SYN_ACK :
      printf("STATE_AWAIT_SYN_ACK\n");

      read_timeout.tv_sec = SHORT_TIMEOUT;
      read_timeout.tv_usec = 0;
    
      while(*state == STATE_AWAIT_SYN_ACK) {

	active_fd = read_fd;
	
	switch(select(sockfd + 1, &active_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  // error
	  perror("STATE_AWAIT_SYN_ACK: Select for syn-ack failed");
	  exit(EXIT_FAILURE);
	case 0:
	  // timeout
	  printf("STATE_AWAIT_SYN_ACK: The server timed out. Returning to STATE_WANT_CONNECT.\n");
	  *state = STATE_WANT_CONNECT;
	  break;
	default:
	  // read
	  retval = recv_rtp(sockfd, &packageReceived, remoteAddr);
	  if (retval < 0) {
	    printf("Checksum error.\n");
	  }
	  else if (packageReceived.flags == FLAG_SYN_ACK && packageReceived.seq == (unsigned) *lastSeqSent) {
	    printf("STATE_AWAIT_SYN_ACK: SYN-ACK received:\n");
	    print_rtp(&packageReceived);
	    *lastSeqReceived = packageReceived.seq;
	    *state = STATE_CONNECTED;
	  }
	  else {
	    printf("STATE_AWAIT_SYN_ACK: Received package that was not SYN-ACK or had incorrect seq.num.. Discarded.\n");
	  }
	}
      }

      if(*state != STATE_CONNECTED)
	break;
      
    case STATE_CONNECTED:
      printf("STATE: STATE_CONNECTED.\n");

      packageToSend.flags = FLAG_ACK;
      packageToSend.seq = *lastSeqReceived;
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, &lastSeqReceived, sizeof(lastSeqReceived));
      
      send_rtp(sockfd, &packageToSend, remoteAddr);
      
      printf("STATE_CONNECTED: SYN-ACK-ACK sent, going to MODE_CONNECTED.\n");
      *mode = MODE_CONNECTED;
      *state = STATE_LISTEN;
    }
  }
  
}
