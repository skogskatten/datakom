#include "functions.h"

/* 1. Byt tillstånd och funktioner. 45%
 * 2. Bygg upp teardown (sender). 30%
 * 3. Sekvensnummer.
 */

int main(int argc, char *argv[]) {
  int sockfd, retval, mode, state;
  int lastSeqSent = 0, lastSeqReceived = 0;
  int timeoutCounter = 0;
  int teardownSenderMode = 1;
  int windowSize = WINDOW_SIZE;
  
  char buffer[MAXLINE];
  char hostName[hostNameLength];
  char *msg = "Msg from the client.", input[5];
  
  rtp packageToSend, packageReceived;
  rtp sendWindow[WINDOW_SIZE];
  
  struct sockaddr_in s_addr, c_addr;
  fd_set read_fd, write_fd, active_fd;
  FD_ZERO(&read_fd);
  FD_ZERO(&write_fd);
  FD_ZERO(&active_fd);
  
  struct timeval read_timeout;
  
  /* Check arguments */
  if(argv[1] == NULL) {
    perror("Usage: client [host name]\n");
    exit(EXIT_FAILURE);
  }
  else {
    strncpy(hostName, argv[1], hostNameLength);
    hostName[hostNameLength - 1] = '\0';
  }
  
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    perror("Exit, server socket creation failure");
    exit(EXIT_FAILURE);
  }

  FD_SET(sockfd, &read_fd);
  FD_SET(sockfd, &write_fd);
  
  
  int s_addrlen = sizeof(s_addr);
  
  memset(&buffer, 0, MAXLINE);
  memset(&s_addr, 0, s_addrlen);
  memset(&c_addr, 0, sizeof(c_addr));

  c_addr.sin_family = AF_INET;
  c_addr.sin_port = htons(PORT);
  c_addr.sin_addr.s_addr = INADDR_ANY;

  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(c_addr.sin_addr), strlen(inet_ntoa(c_addr.sin_addr)));

  struct hostent *hostInfo; /* Contains info about the host */

  hostInfo = gethostbyname(hostName);
  if(hostInfo == NULL) {
    fprintf(stderr, "initSocketAddress - Unknown host %s\n", hostName);
    exit(EXIT_FAILURE);
  }
  
  s_addr.sin_addr = *(struct in_addr *)hostInfo->h_addr;
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(PORT);


  /*  BEGINNING OF STATES  */
  
  mode = MODE_AWAIT_CONNECT;
  state = STATE_WANT_CONNECT;
  
  switch(mode) {
  default :
    printf("Undefined case: mode . Exiting.\n");
    exit(EXIT_FAILURE);

  case MODE_AWAIT_CONNECT:

    ConnectionSender(&state, &mode, sockfd, read_fd, write_fd, &lastSeqReceived, &lastSeqSent, windowSize, &s_addr, &c_addr);

    retval = ZeroWindow(sendWindow, windowSize);
    printf("MODE_CONNECTED: Zero'd %d slots.\n", retval);
    
  case MODE_CONNECTED :
    printf("MODE: MODE_CONNECTED.\n");
    while (mode == MODE_CONNECTED) {

      switch(state) {
      case STATE_LISTEN:
	printf("Client, going into SlidingReceiver().\n");
	SlidingReceiver(&timeoutCounter, &state, &mode, sockfd, sockfd, read_fd, &lastSeqReceived, &lastSeqSent, windowSize, sendWindow, &s_addr, &c_addr, &teardownSenderMode);
	break;
	
      case STATE_SEND:
	printf("Client, going into SlidingSender().\n");
	SlidingSender(msg, &timeoutCounter, &state, &mode, sockfd, sockfd, read_fd, write_fd, &lastSeqReceived, &lastSeqSent, windowSize, sendWindow, &s_addr, &c_addr);
	      
	printf("Continue?");
	scanf("%s", input);
	if(strcmp("FIN", input) == 0) {
	  mode = MODE_TEARDOWN;
	}
	else
	  state = STATE_LISTEN;
      }
    }

  case MODE_TEARDOWN :
    printf("MODE_TEARDOWN\n");
    if (teardownSenderMode) {
      printf("Client, going into TeardownSender.\n");
      TeardownSender(&state, &mode, sockfd, sockfd, write_fd, read_fd, &lastSeqSent, &lastSeqReceived, sendWindow, &s_addr, &c_addr);
    }
    else {
      printf("Client, going into TeardownReceiver.\n");
      TeardownReceiver(&state, &mode, sockfd, sockfd, write_fd, read_fd, &lastSeqSent, &lastSeqReceived, &s_addr, &c_addr);
    }
  }

  //  usleep(50000);
  exit(EXIT_SUCCESS);

}


