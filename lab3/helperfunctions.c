#include "functions.h"


/* Helper functions */

void CleanRtpData(rtp *toClean) {
  memset(toClean->data, '\0', MAX_DATA_LEN);
}

int IsWindowFull(rtp *window, int windowSize) {
  for(int i = 0; i < windowSize; i++)
    if (window[i].seq == 0)
      return 0;

  return 1;
}

int AddToWindow(rtp *window, int windowSize, rtp toAdd) {
  for(int i = 0; i < windowSize; i++)
    if (window[i].seq == 0) {
      window[i] = toAdd;
      return 0;
    }
  return -1;
}

int RemoveFromWindow(rtp *window, int windowSize, unsigned int seqToRemove) {
  int i = 0;
  while (window[i].seq != seqToRemove) {
    i++;
    if (i == windowSize)
      return -1;
  }
  
  while ( i + 1 < windowSize ) {
    window[i] = window[i + 1];
    i++;
  }
  window[i].seq = 0;
  
  return 0;
}

int RemoveAcknowledgedFromWindow(rtp *window, int windowSize, unsigned int ackedSeq) {
  int numRemoved = 0;
  for(int i = 0; i < windowSize; i++) {
    if (window[i].seq <= ackedSeq) {
      RemoveFromWindow(window, windowSize, window[i].seq);
      numRemoved++;
    }
  }
  return numRemoved;
}

rtp GetFromWindow(rtp *window, int windowSize, unsigned int seqToGet) {
  int i = 0;
  while (window[i].seq != seqToGet) {
    i++;
    if (i == windowSize) {
      rtp emptyReturn;
      emptyReturn.seq = 0;
      return emptyReturn;
    }
  }
  return window[i];
}

int ResendWindow(rtp *window, int windowSize, int sockfd, struct sockaddr_in *remoteAddr) {
  int numSent = 0;
  fd_set write_fd, active_fd;
  FD_ZERO(&active_fd);
  FD_SET(sockfd, &active_fd);
  
  for(int i = 0; i < windowSize; i++) {
    if (window[i].seq == 0)
      break;
    write_fd = active_fd;
    switch(select(sockfd + 1, NULL, &write_fd, NULL, NULL)) {
    case -1:
      {
	perror("ResendWindow");
	break;
      }
    default:
      send_rtp(sockfd, &window[i], remoteAddr);
      numSent++;
    }
  }
  return numSent;
}

rtp *AllocateWindow(int windowSize) {
  rtp *window = malloc(sizeof(rtp)*windowSize);
  if(window == NULL) {
    perror("AllocateWindow");
    exit(EXIT_FAILURE);
  }
  return window; 
}
