#include "functions.h"

void SlidingSender(char *msg, int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, fd_set write_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr) {

  PrintWindow(sendWindow, windowSize);
  if(IsWindowFull(sendWindow, windowSize)) {
    printf("SlidingSender: Sender window is full.\n");
    return;
  }

  struct timeval write_timeout;
  write_timeout.tv_sec = MEDIUM_TIMEOUT;
  write_timeout.tv_usec = 0;
  
  fd_set active_fd;
  FD_ZERO(&active_fd);
  active_fd = write_fd;
  *state = STATE_LISTEN;
  switch(select(writeSock + 1, NULL, &active_fd, NULL, &write_timeout)) {
  case -1: {
    perror("SlidingSender: Select:");
    exit(EXIT_FAILURE);
  }
  case 0: {
    printf("SlidingSender: Select timeout. The socket is busy.\n");
    *state = STATE_LISTEN;
    return;
  }
  default: {

    rtp packageToSend;
    memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(localAddr->sin_addr), strlen(inet_ntoa(localAddr->sin_addr)));
    packageToSend.flags = FLAG_DATA;
    (*lastSeqSent)++;
    packageToSend.seq = *lastSeqSent;
    CleanRtpData(&packageToSend);
    memcpy(&packageToSend.data, msg, strlen(msg));

    // add to sender window
    AddToWindow(sendWindow, windowSize, packageToSend);
    
    printf("SlidingSender: Sending:\n");
    print_rtp_header();
    print_rtp(&packageToSend);
    
    send_rtp(writeSock, &packageToSend, remoteAddr);
    *state = STATE_LISTEN;
  }
  }
  *state = STATE_LISTEN;

}
