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


