#include "functions.h"

/* 1. Byt tillstånd och funktioner. 45%
 * 2. Bygg upp teardown (sender). 30%
 * 3. Sekvensnummer.
 */

int main(int argc, char *argv[]) {
  int sockfd, retval, mode, state;
  int lastSeqSent = 0, lastSeqReceived = 0;
  int timeoutCounter = 0;
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
  
  memcpy((void *)&packageToSend.id, (const void *)&c_addr, sizeof(c_addr));

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
    printf("MODE: unconnected.\n");

    switch(state) {
    default :
      printf("Undefined case: state . Exiting.\n");
      exit(EXIT_FAILURE);

      
    case STATE_WANT_CONNECT :
      printf("STATE: wants to connect.\n");
      
      packageToSend.flags = FLAG_SYN;
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, &windowSize, sizeof(windowSize));
      memcpy(&packageToSend.windowsize, &windowSize, sizeof(windowSize)); // This will inform the server of the window size we will use.
      packageToSend.seq = lastSeqSent++;
      
      send_rtp(sockfd, &packageToSend, &s_addr);
            
      printf("Sent:\n");
      print_rtp(&packageToSend);
      
      state = STATE_AWAIT_SYN_ACK;

      
    case STATE_AWAIT_SYN_ACK :
      printf("STATE: wait for syn-ack.\n");

      while(state == STATE_AWAIT_SYN_ACK) {
	
	read_timeout.tv_sec = SHORT_TIMEOUT;
	read_timeout.tv_usec = 0;
	active_fd = read_fd;
	
	switch(select(sockfd + 1, &active_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  // error
	  perror("MODE_CONNECT: Select for syn-ack failed");
	  exit(EXIT_FAILURE);
	case 0:
	  // timeout
	  printf("The server timed out. Returning to STATE_WANT_CONNECT.\n");
	  state = STATE_WANT_CONNECT;
	  break;
	default:
	  // read
	  retval = recv_rtp(sockfd, &packageReceived, &s_addr);
	  if (retval < 0) {
	    printf("Checksum error.\n");
	  }
	  else if (packageReceived.flags == FLAG_SYN_ACK) {
	    printf("STATE_AWAIT_SYN_ACK: SYN-ACK received:\n");
	    print_rtp(&packageReceived);
	    state = STATE_CONNECTED;
	  }
	  else {
	    printf("Received: ");
	    print_rtp(&packageReceived);
	    printf("Still waiting for SYN-ACK.");
	  }
	}
      }

      
    case STATE_CONNECTED:
      printf("STATE: STATE_CONNECTED.\n");

      packageToSend.flags = FLAG_ACK;
      CleanRtpData(&packageToSend);
      packageToSend.seq = lastSeqSent++;
      memcpy(&packageToSend.data, &lastSeqReceived, sizeof(lastSeqReceived));
      send_rtp(sockfd, &packageToSend, &s_addr);
      printf("STATE_CONNECTED: SYN-ACK-ACK sent, going to MODE_CONNECTED.\n");
      mode = MODE_CONNECTED;
    }

  case MODE_CONNECTED :
    printf("MODE: MODE_CONNECTED.\n");
    while (mode == MODE_CONNECTED) {

      switch(state) {
      case STATE_LISTEN:
	SlidingReceiver(&timeoutCounter, &state, &mode, sockfd, sockfd, read_fd, &lastSeqReceived, &lastSeqSent, windowSize, sendWindow, &s_addr, &c_addr);
	break;
	
      case STATE_SEND:
	SlidingSender(msg, &timeoutCounter, &state, &mode, sockfd, sockfd, read_fd, write_fd, &lastSeqReceived, &lastSeqSent, windowSize, sendWindow, &s_addr, &c_addr);
	      
	printf("Continue?");
	scanf("%s", input);
	if(strcmp("FIN", input) == 0) {
	  mode = MODE_TEARDOWN;
	}
      }
    }

  case MODE_TEARDOWN :
    
    TeardownSender(&state, &mode, sockfd, sockfd, write_fd, read_fd, &lastSeqSent, &lastSeqReceived, sendWindow, &s_addr, &c_addr);
    
  }

  usleep(50000);
  exit(EXIT_SUCCESS);

}


