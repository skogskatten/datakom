#include "functions.h"



void TeardownSender(int *state, int *mode, int writeSock, int readSock, fd_set write_fd, fd_set read_fd, int *lastSeqSent, int *lastSeqReceived, rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {

  int retval, timeoutCounter = 0;

  rtp packageToSend, packageReceived;
  memcpy((void *)&packageToSend.id, (const void *)localAddr, sizeof(*localAddr));
  
  struct timeval read_timeout;
  
  fd_set active_fd;
  FD_ZERO(&active_fd);
  
  printf("TeardownSender: MODE_TEARDOWN.\n");
  while(*mode == MODE_TEARDOWN) {
    switch(*state) {
    case STATE_CONNECTED: {

      packageToSend.flags = FLAG_FIN;
      (*lastSeqSent)++;
      packageToSend.seq = *lastSeqSent;
      CleanRtpData(&packageToSend);
      printf("TeardownSender: STATE_CONNECTED: Sending FIN:\n");
      print_rtp_header();
      print_rtp(&packageToSend);
      send_rtp(writeSock, &packageToSend, remoteAddr);
      
      *state = STATE_AWAIT_FIN_ACK;
      break;
    }
      
    case STATE_AWAIT_FIN_ACK: {
      
      printf("TeardownSender: STATE_AWAIT_FIN_ACK: FIN sent, awaiting FIN-ACK.\n");

      /* This timeout is placed outside the loop so that the total listening time will not exceed the timeout time. This is because of how the gnu/linux select functions. (see select(2)) */
      read_timeout.tv_sec = LONG_TIMEOUT*2;
      read_timeout.tv_usec = 0;
      timeoutCounter = 0;
      
      while(*state == STATE_AWAIT_FIN_ACK) {

	//	active_fd = read_fd;
	FD_SET(readSock, &active_fd);
	sleep(1);
	retval = select(readSock + 1, &active_fd, NULL, NULL, &read_timeout);
	switch(retval) {
	case -1 :
	  perror("Client, select");
	  exit(EXIT_FAILURE);
	case 0 :
	  printf("Timeout, going to timeout state.\n");
	  *state = STATE_TIMEOUT;
	  break;
	default : {

	  retval = recv_rtp(readSock, &packageReceived, remoteAddr);
	  if (retval < 0) {
	    printf("STATE_AWAIT_FIN_ACK: Received msg, checksum error.\n");
	    break;
	  }

	  printf("Received: ");
	  print_rtp(&packageReceived);
	  
	  if (packageReceived.flags == FLAG_FIN_ACK) {
	    printf("STATE_AWAIT_FIN_ACK: Received FIN-ACK.\n");
	    *state = STATE_AWAIT_FIN;
	    break;
	  }
	  else if (packageReceived.flags == FLAG_FIN) {
	    printf("STATE_AWAIT_FIN_ACK: Received FIN. Listening for FIN-ACK and then moving to STATE_TIMEOUT.\n");
	    // SENDING FIN-ACK
	    packageToSend.flags = FLAG_FIN_ACK;
	    packageToSend.seq = packageReceived.seq;
	    CleanRtpData(&packageToSend);
	    memcpy(&packageToSend.data, &packageReceived.seq, sizeof(packageReceived.seq));

	    send_rtp(writeSock, &packageToSend, remoteAddr);

	    // WAITING FOR FIN-ACK
	    /* See select(2) for timeout placement. */
	    read_timeout.tv_sec = LONG_TIMEOUT;
	    read_timeout.tv_usec = 0;
	  
	    while (*state == STATE_AWAIT_FIN_ACK) {
	    
	      active_fd = read_fd;
	      
	      retval = select(readSock + 1, &active_fd, NULL, NULL, &read_timeout);
	      switch(retval) {
	      case -1 :
		perror("STATE_AWAIT_FIN_ACK, 2nd select for FIN-ACK after FIN");
		exit(EXIT_FAILURE);
	      case 0 :
		printf("Timeout, going to timeout state.\n");
		*state = STATE_TIMEOUT;
		break;
	      default : 
		retval = recv_rtp(readSock, &packageReceived, remoteAddr);
		if (retval < 0) {
		  printf("Checksum error.\n");
		  break;
		}
		else if (packageReceived.flags != FLAG_FIN_ACK){
		  printf("STATE_AWAIT_FIN_ACK, 2nd select for FIN-ACK after FIN: msg received is not relevant.\n");
		  break;
		}
	      }
	    }
	  }
	  else {
	    printf("Msg is not relevant. Discarding.\n");
	    break;
	  }
	}
	}
      }
      break;
    }
   
    case STATE_AWAIT_FIN : {
    
      printf("STATE: FIN-ACK received, waiting for FIN.\n");
      read_timeout.tv_sec = LONG_TIMEOUT;
      read_timeout.tv_usec = 0;

      while (*state == STATE_AWAIT_FIN) {
	//	active_fd = read_fd;
    	FD_SET(readSock, &active_fd);
	retval = select(readSock + 1, &active_fd, NULL, NULL, &read_timeout);
	switch(retval) {
	case -1 :
	  perror("Client, select");
	  exit(EXIT_FAILURE);
	case 0 :
	  printf("Timeout, going to timeout state.\n");
	  *state = STATE_TIMEOUT;
	  break;
	default :
      
	  retval = recv_rtp(readSock, &packageReceived, remoteAddr);
	  if(retval < 0) {
	    printf("STATE_AWAIT_FIN: Bad checksum.\n");
	  }
	  else if (packageReceived.flags == FLAG_FIN) {
	    printf("STATE_AWAIT_FIN: FIN received, sending FIN-ACK.\n");

	    packageToSend.flags = FLAG_FIN_ACK;
	    packageToSend.seq = packageReceived.seq;
	    CleanRtpData(&packageToSend);
	    memcpy(&packageToSend.data, &packageReceived.seq, sizeof(packageReceived.seq));

	    send_rtp(writeSock, &packageToSend, remoteAddr);

	    printf("STATE_AWAIT_FIN: Going to timeout.\n");
	    *state = STATE_TIMEOUT;
	  }
	  else {
	    printf("STATE_AWAIT_FIN: Msg is not relevant.\n");
	    break;
	  }
	}
      }
      break;
    }
    
    case STATE_TIMEOUT: {
      printf("TeardownSender: STATE_TIMEOUT\n");
      usleep(50000);
      close(readSock);
      *state = STATE_DISCONNECTED;
      break;
    }
    case STATE_DISCONNECTED: {
    
      return;
    }
    }
  }
}



