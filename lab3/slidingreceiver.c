#include "functions.h"

void SlidingReceiver(int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, int *lastSeqReceived, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {
  //Receives packet out of order -> resend last ack
  //Receives packet in order -> send new ACK
  // (*lastSeqReceived)++;
  rtp packageReceived, packageToSend;
  memcpy((void *)&packageToSend.id, (const void *)localAddr, sizeof(*localAddr));
  
  struct timeval read_timeout;
  fd_set active_fd;
  FD_ZERO(&active_fd);
  
  read_timeout.tv_sec = SHORT_TIMEOUT;
  read_timeout.tv_usec = 0;
  
  active_fd = read_fd;
	
  switch(select(readSock + 1, &active_fd, NULL, NULL, &read_timeout)) {
  case -1:
    perror("MODE_CONNECTED: Select listening for incoming msgs failed");
    exit(EXIT_FAILURE);
    
  case 0:
    printf("MODE_CONNECTED: ");
    (*timeoutCounter)++;
    if (*timeoutCounter >= 5 && *timeoutCounter < 10) {
      // resend last package
    }
    else if (*timeoutCounter >= 10) {
      printf("MODE_CONNECTED: Connection timed out; nothing to read. Going to MODE_TEARDOWN (sender).\n");
      *mode = MODE_TEARDOWN;
    }
    *state = STATE_SEND;
    break;
    
  default:
    *timeoutCounter = 0;
    if(recv_rtp(readSock, &packageReceived, remoteAddr) < 0) {
      printf("MODE_CONNECTED: Received, checksum failed.");
      break;
    }
    
    if (packageReceived.flags != FLAG_ACK) {
      
      // receives packet in order, send ack N
      if (packageReceived.seq == (unsigned int) (*lastSeqReceived) + 1) {
	(*lastSeqReceived)++;
	printf("Received: ");
	print_rtp(&packageReceived);
      }
      else {
	printf("Package received out of order. Discarding package.\n");
      }
    

      // receives packet out of order, resend last ACK (latest received seq.num.).
      // ACKar ökar inte lastseqsent
    
      packageToSend.flags = FLAG_ACK;
      packageToSend.seq = (*lastSeqReceived);
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, lastSeqReceived, sizeof(*lastSeqReceived));

      printf("Sending ACK:\n");
      print_rtp(&packageToSend);
    
      send_rtp(writeSock, &packageToSend, remoteAddr);
    
    }
    else { // If ack
      /* Ta bort varje paket med seq <= ACKad seq från fönster */
      printf("SlidingReceiver: Removed %d packages from the sender window.\n", RemoveAcknowledgedFromWindow(sendWindow, windowSize, packageReceived.seq));
    }
    
  }
}
