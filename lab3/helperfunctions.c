#include "functions.h"


/* Helper functions */

void CleanRtpData(rtp *toClean) {
  memset(toClean->data, '\0', MAX_DATA_LEN);
}

void PrintWindow(rtp *window, int windowSize) {
  printf("PrintWindow():\n");
  print_rtp_header();
  for (int i = 0; i < windowSize; i++) {
    if (window[i].seq != 0)
      print_rtp(&window[i]);
  }
}

int IsWindowFull(rtp *window, int windowSize) {
  for(int i = 0; i < windowSize; i++) {
    //printf("IsWindowfull: window[%d].seq == %d.\n", i, window[i].seq);
    if (window[i].seq == 0) {
      return 0;
    }
  }
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
  
  /* while ( i + 1 < windowSize ) { */
  /*   window[i] = window[i + 1]; */
  /*   i++; */
  /* } */
  
  window[i].seq = 0;
  
  return 0;
}

int RemoveAcknowledgedFromWindow(rtp *window, int windowSize, unsigned int ackedSeq) {
  int numRemoved = 0;
  for(int i = 0; i < windowSize; i++) {
    if (window[i].seq <= ackedSeq && window[i].seq > 0) { // Här blir det konstigt.
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

int IsInWindow(rtp *window, int windowSize, unsigned int seqToFind) {
  for (int i = 0; i < windowSize; i++) {
    if (window[i].seq == seqToFind)
      return 1;
  }
  return 0;
}

int ResendWindow(rtp *window, int windowSize, int sockfd, struct sockaddr_in *remoteAddr) {
  int numSent = 0;
  fd_set write_fd, active_fd;
  FD_ZERO(&active_fd);
  FD_SET(sockfd, &active_fd);
  
  for(int i = 0; i < windowSize; i++) {
    if (window[i].seq == 0)
      continue;
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

int ZeroWindow(rtp *window, int windowSize) {
  int numZerod = 0;
  for(int i = 0; i < windowSize; i++) {
    window[i].flags = '0';
    window[i].id = '0';
    window[i].seq = 0;
    window[i].windowsize = '0';
    memcpy(window[i].data, "NULL", strlen("NULL"));
    window[i].crc = '0';
    numZerod++;
  }
  return numZerod;
}

rtp *AllocateWindow(int windowSize) {
  rtp *window = malloc(windowSize * sizeof(*window));
  printf("AllocateWindow: sizeof(*window) returns %d.\n", (unsigned)sizeof(*window));
  printf("AllocateWindow: sizeof(rtp) returns %d.\n", (unsigned)sizeof(rtp));
  if(window == NULL) {
    perror("AllocateWindow");
    exit(EXIT_FAILURE);
  }
  int i;
  for(i = 0; i < windowSize; i++) {
    window[i].crc = '0';
    window[i].id = '0';
    window[i].seq = 0;
    window[i].windowsize = '0';
    window[i].data[0] = '\0';
    window[i].crc = '0';
    //window[i].seq = 0;
    //printf("AllocateWindow: Zeroed slot %d. window[%d].seq == %d.\n", i, i, window[i].seq);
  }
  printf("AllocateWindow: Zeroed %d slots.\n", (i));
  return window; 
}
