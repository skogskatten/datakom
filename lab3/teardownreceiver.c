#include "functions.h"


void TeardownReceiver(int *state, int *mode, int writeSock, int readSock, fd_set write_fd, fd_set read_fd, int *lastSeqSent, int *lastSeqReceived, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {
  
  int retval;

  rtp packageToSend, packageReceived;
  memcpy((void *)&packageToSend.id, (const void *)localAddr, sizeof(*localAddr));
  
  struct timeval read_timeout;
  
  fd_set active_fd;
  FD_ZERO(&active_fd);
  
  while(*mode == MODE_TEARDOWN) {
  
    switch(*state) {
    default: {
      printf("MODE_TEARDOWN unknown state.");
      *state = STATE_TIMEOUT;
      break;
    }
    case STATE_CONNECTED: {

      // FIN received. Send ack.
      packageToSend.flags = FLAG_FIN_ACK;
      packageToSend.seq = *lastSeqReceived;
      memset(&packageToSend.data, '\0', MAX_DATA_LEN);
      printf("TeardownReceiver: STATE_CONNECTED: FIN received, sending FIN-ACK:\n");
      print_rtp_header();
      print_rtp(&packageToSend);
      send_rtp(writeSock, &packageToSend, remoteAddr);

      *state = STATE_SHUTTING_DOWN;

      break;
    }
    
    case STATE_SHUTTING_DOWN: {
      
      //Send FIN.
      sleep(2);
      packageToSend.flags = FLAG_FIN;
      (*lastSeqSent)++;
      packageToSend.seq = *lastSeqSent;
    
      memcpy(&packageToSend.data, &packageToSend.seq, sizeof(packageToSend.seq));
      printf("TeardownReceiver: STATE_SHUTTING_DOWN: Sending FIN: \n");
      print_rtp_header();
      print_rtp(&packageToSend);
      send_rtp(writeSock, &packageToSend, remoteAddr);

      *state = STATE_AWAIT_FIN_ACK;
      break;
    }
    case STATE_AWAIT_FIN_ACK: {
      printf("TeardownReceiver: STATE_AWAIT_FIN_ACK.\n");
      //select med timeout
	
      read_timeout.tv_sec = LONG_TIMEOUT*2;
      read_timeout.tv_usec = 0;
    
      while(*state == STATE_AWAIT_FIN_ACK) {
	active_fd = read_fd;
    
	switch(select(readSock + 1, &active_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  perror("STATE_AWAIT_FIN_ACK");
	  *state = STATE_DISCONNECTED;
	  break;
	case 0:
	  printf("STATE_AWAIT_FIN_ACK: timeout, disconnecting.\n");
	  *state = STATE_DISCONNECTED;
	  break;
	default:
	  retval = recv_rtp(writeSock, &packageReceived, remoteAddr);
	  if(retval < 0)
	    printf("STATE_AWAIT_FIN_ACK: Checksum error.\n");
	  else if(packageReceived.flags == FLAG_FIN_ACK) {
	    printf("STATE_AWAIT_FIN_ACK: FIN-ACK received, disconnecting.\n");
	    *state = STATE_DISCONNECTED;
	  }
	  else
	    printf("STATE_AWAIT_FIN_ACK: Received non-FIN-ACK message. Discarding.\n");
	      
	}
      }
      break;
    }
    
    case STATE_DISCONNECTED: {
      printf("TeardownReceiver: STATE_DISCONNECTED\n");
      close(readSock);
      return;
    }
    }
  }
}
