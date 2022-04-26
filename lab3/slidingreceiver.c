#include "functions.h"

void SlidingReceiver(int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr, int *teardownSenderMode) {
  
  //Receives packet out of order -> resend last ack
  //Receives packet in order -> send new ACK
  // (*lastSeqReceived)++;
  
  rtp packageReceived, packageToSend;
  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(localAddr->sin_addr), strlen(inet_ntoa(localAddr->sin_addr)));
  
  fd_set active_fd;
  FD_ZERO(&active_fd);

  struct timeval read_timeout;
  
  read_timeout.tv_sec = SHORT_TIMEOUT;
  read_timeout.tv_usec = 0;
  
  active_fd = read_fd;
	
  switch(select(readSock + 1, &active_fd, NULL, NULL, &read_timeout)) {
  case -1: {
    perror("MODE_CONNECTED: Select listening for incoming msgs failed");
    exit(EXIT_FAILURE);
  }
  case 0: { // If timeout
    printf("MODE_CONNECTED: ");
    (*timeoutCounter)++;
    if (*timeoutCounter >= 5 && *timeoutCounter < 10) {
      
      // if fifth timeout -> resend last ACK
      /* packageToSend = GetFromWindow(sendWindow, windowSize, *lastSeqSent); */
      /* if(packageToSend.seq == 0) */
      /* 	break; */
      printf("SlidingReceiver: Timeout 5, resent %d packages from window.\n", ResendWindow(sendWindow, windowSize, writeSock, remoteAddr));
      
    }
    else if (*timeoutCounter >= 10) {
      // if 10th timeout -> go to teardown
      printf("MODE_CONNECTED: Connection timed out; nothing to read. Going to MODE_TEARDOWN (sender), STATE_SHUTTING_DOWN.\n");
      
      *mode = MODE_TEARDOWN;
      *state = STATE_CONNECTED;
      *teardownSenderMode = 1;
      
      break;
    }
  
    *state = STATE_SEND;
    break;
  }
  default: {// If message found.
    *timeoutCounter = 0; // timeout is reset since there is communication
    
    if(recv_rtp(readSock, &packageReceived, remoteAddr) < 0) {
      printf("MODE_CONNECTED: Received, checksum failed.\n");
      *state = STATE_SEND;
      return;
    }
    
    if (packageReceived.flags != FLAG_ACK && packageReceived.flags != FLAG_FIN) {
      // receives packet in order, send ack N
      if (packageReceived.seq == (unsigned int) (*lastSeqReceived) + 1) {
	(*lastSeqReceived)++;
	printf("SlidingReceiver: Received: \n");
	print_rtp_header();
	print_rtp(&packageReceived);
      }
      else {
	printf("SlidingReceiver: Package received out of order: ");
	print_rtp_header();
	print_rtp(&packageReceived);
	printf("SlidingReceiver: Discarding package and resending last ACK.\n");
      }
    

      // receives packet out of order, resend last ACK (latest received seq.num.).
      // ACKar ökar inte lastseqsent
    
      packageToSend.flags = FLAG_ACK;
      packageToSend.seq = (*lastSeqReceived);
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, lastSeqReceived, sizeof(*lastSeqReceived));
      
      printf("SlidingReceiver: Sending ACK:\n");
      print_rtp_header();
      print_rtp(&packageToSend);
      
      send_rtp(writeSock, &packageToSend, remoteAddr);
      
    }
    else if (packageReceived.flags == FLAG_FIN) {
      *teardownSenderMode = 0;
      *lastSeqReceived = packageReceived.seq;
      printf("SlidingReceiver: FIN received. Going to teardown (receiver).\n");
      *mode = MODE_TEARDOWN;
      *state = STATE_CONNECTED;
    }
    else { // If ack
      
      /* Ta bort varje paket med seq <= ACKad seq från fönster */
      if (IsInWindow(sendWindow, windowSize, packageReceived.seq)) {
	printf("SlidingReceiver: Received ACK with seq.num. %d. Removed %d packages from the sender window.\n", packageReceived.seq, RemoveAcknowledgedFromWindow(sendWindow, windowSize, packageReceived.seq));
      }
      else {
	printf("SlidingReceiver: Received old seq.num. which means package was lost, resends %d packages from window.\n", ResendWindow(sendWindow, windowSize, writeSock, remoteAddr));
      }
    }
  }
  }
  if(*state == STATE_LISTEN)
    *state = STATE_SEND;
}
