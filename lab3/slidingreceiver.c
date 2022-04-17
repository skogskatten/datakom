#include "functions.h"

void SlidingReceiver(int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {
  
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
  case -1:
    perror("MODE_CONNECTED: Select listening for incoming msgs failed");
    exit(EXIT_FAILURE);
    
  case 0: // If timeout
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
      *state = STATE_SHUTTING_DOWN; // This is the state where the server sends a FIN, waits for an ACK and receives or times out, then disconnects.
      
      break;
    }
    
    *state = STATE_SEND;
    break;
    
  default: // If message found.
    *timeoutCounter = 0; // timeout is reset since there is communication
    
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
      printf("SlidingReceiver: Received ACK with seq.num. %d. Removed %d packages from the sender window.\n", packageReceived.seq, RemoveAcknowledgedFromWindow(sendWindow, windowSize, packageReceived.seq));
    }
  }
  
  if(*state == STATE_LISTEN)
    *state = STATE_SEND;
}
