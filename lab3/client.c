#include "functions.h"
/* 1. Byt tillstånd och funktioner. 45%
 * 2. Bygg upp teardown (sender). 30%
 * 3. Sekvensnummer.
 */
#define PORT 5555
#define MAXLINE 1024
#define hostNameLength 50
#define WINDOW_SIZE 5
#define RESET -1
#define SHORT_TIMEOUT 1
#define MEDIUM_TIMEOUT 5
#define LONG_TIMEOUT 10

void CleanRtpData(rtp *toClean);

int main(int argc, char *argv[]) {
  int sockfd, retval, mode, state;
  int lastSeqSent = -1, lastSeqReceived = -1;
  int timeoutCounter = 0;
  char buffer[MAXLINE];
  char hostName[hostNameLength];
  char *msg, windowSize;
  rtp packageToSend, packageReceived;
  
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

  FD_SET(sockfd, &active_fd);
  
  
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
      windowSize =  WINDOW_SIZE;
      CleanRtpData(&packageToSend);
      memcpy(&packageToSend.data, &windowSize, sizeof(windowSize));
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
	read_fd = active_fd;
	
	switch(select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout)) {
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
	read_timeout.tv_sec = SHORT_TIMEOUT;
	read_timeout.tv_usec = 0;
	read_fd = active_fd;
	
	switch(select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  perror("MODE_CONNECTED: Select listening for incoming msgs failed");
	  exit(EXIT_FAILURE);
	case 0:
	  printf("MODE_CONNECTED: ");
	  timeoutCounter++;
	  if (timeoutCounter >= 5 && timeoutCounter < 10) {
	    // resend last package
	  }
	  else if (timeoutCounter >= 10) {
	    printf("MODE_CONNECTED: Connection timed out, nothing to read. Going to MODE_TEARDOWN (sender state).\n");
	    mode = MODE_TEARDOWN;
	  }
	  state = STATE_SEND;
	  break;
	default:
	  timeoutCounter = 0;
 	  if(recv_rtp(sockfd, &packageReceived, &s_addr) < 0) {
	    printf("MODE_CONNECTED: Received, checksum failed.");
	    break;
	  }

	  // if (packageReceived.seq = lastSeqReceived + 1) {lastSeqReceived++}
	  // receives packet in order, send ack N
	  printf("Received: ");
	  print_rtp(&packageReceived);

	  // receives packet out of order, resend last ACK (latest received seq.num.).

	  packageToSend.flags = FLAG_ACK;
	  packageToSend.seq = lastSeqSent++; // Är det här lämpligt? Skall en ack öka seq?
	  CleanRtpData(&packageToSend);
	  memcpy(&packageToSend.data, &lastSeqReceived, sizeof(lastSeqReceived));
	  send_rtp(sockfd, &packageToSend, &s_addr);
	  
	}
      case STATE_SEND:
	packageToSend.flags = FLAG_DATA;
	packageToSend.seq = lastSeqSent++;
	CleanRtpData(&packageToSend);
	memcpy(&packageToSend.data, "Message from the client.", strlen("Message from the client."));
	send_rtp(sockfd, &packageToSend, &s_addr);
	// add to sender window
	state = STATE_LISTEN;
      }
    }





    
    /* Just nu faller den igenom alla STATE_TIMEOUT. behöver while loop alt. ex if-else för att skippa fall då gått till timeout */  
  case MODE_TEARDOWN :
    printf("MODE: MODE_TEARDOWN.\n");
    switch(state) {

      
    case STATE_CONNECTED:

      packageToSend.flags = FLAG_FIN;
      packageToSend.seq = lastSeqSent++;
      CleanRtpData(&packageToSend);
      
      send_rtp(sockfd, &packageToSend, &s_addr);
      
      state = STATE_AWAIT_FIN_ACK;
      
      
    case STATE_AWAIT_FIN_ACK:
      
      printf("STATE: FIN sent, awaiting ack.\n");
      
      while(state == STATE_AWAIT_FIN_ACK) {
	
	read_timeout.tv_sec = MEDIUM_TIMEOUT;
	read_timeout.tv_usec = 0;
	read_fd = active_fd;
      
	retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
	switch(retval) {
	case -1 :
	  perror("Client, select");
	  exit(EXIT_FAILURE);
	case 0 :
	  printf("Timeout, going to timeout state.\n");
	  state = STATE_TIMEOUT;
	  break;
	default :

	  retval = recv_rtp(sockfd, &packageReceived, &s_addr);
	  if (retval < 0) {
	    printf("STATE_AWAIT_FIN_ACK: Received msg, checksum error.\n");
	    break;
	  }

	  printf("Received: ");
	  print_rtp(&packageReceived);
	  
	  if (packageReceived.flags == FLAG_FIN_ACK) {
	    printf("STATE_AWAIT_FIN_ACK: Received SYN-ACK.\n");
	    state = STATE_AWAIT_FIN;
	    break;
	  }
	  else if (packageReceived.flags == FLAG_FIN) {
	    printf("STATE_AWAIT_FIN_ACK: Received FIN. Listening for FIN-ACK and then moving to STATE_TIMEOUT.\n");
	    packageToSend.flags = FLAG_FIN_ACK;
	    packageToSend.seq = lastSeqSent++;
	    CleanRtpData(&packageToSend);
	    memcpy(&packageToSend.data, &packageReceived.seq, sizeof(packageReceived.seq));

	    send_rtp(sockfd, &packageToSend, &s_addr);

	    while (state == STATE_AWAIT_FIN_ACK) {
	    
	      read_timeout.tv_sec = MEDIUM_TIMEOUT;
	      read_timeout.tv_usec = 0;
	      read_fd = active_fd;
	      
	      retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
	      switch(retval) {
	      case -1 :
		perror("STATE_AWAIT_FIN_ACK, 2nd select for FIN-ACK after FIN");
		exit(EXIT_FAILURE);
	      case 0 :
		printf("Timeout, going to timeout state.\n");
		state = STATE_TIMEOUT;
		break;
	      default :
		retval = recv_rtp(sockfd, &packageReceived, &s_addr);
		if (retval < 0) {
		  printf("Checksum error.\n");
		  break;
		}

		if()
		
	      }
	    }
	    
	  }
	  else {
	    printf("Msg is not relevant. Discarding.\n");
	    break;
	  }

	  /* else if mottar fin, vänta på finack/timeout och gå direkt till timeout.*/

	}
      }
    case STATE_AWAIT_FIN :
      printf("STATE: FIN-ACK received, waiting for FIN.\n");
      read_timeout.tv_sec = MEDIUM_TIMEOUT;
      read_timeout.tv_usec = 0;
      read_fd = active_fd;
      
      retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
      switch(retval) {
      case -1 :
	perror("Client, select");
	exit(EXIT_FAILURE);
      case 0 :
	printf("Timeout, going to timeout state.\n");
	state = STATE_TIMEOUT;
	break;
      default :
	if(recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, (unsigned int *)&s_addrlen) < 0) {
	  perror("Client, receiving fin");
	  exit(EXIT_FAILURE);
	}
	printf("Received fin.\n");

	// Send ack
      }
      
    case STATE_TIMEOUT:
      usleep(500000);
      close(sockfd);

    case STATE_DISCONNECTED:
      break;
    }
  }

  usleep(50000);
  exit(EXIT_SUCCESS);

}


void CleanRtpData(rtp *toClean) {
  memset(toClean->data, '\0', MAX_DATA_LEN);
}
