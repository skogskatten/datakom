#include "functions.h"
/* Att göra
 * 1. Ordna upp tillstånd. Lägg ev. till något om det saknas.
 * 2. Testa om fortfarande fungerar som server3 gjorde.
 * 3. Implementera checksummehantering.
 * 4. Implementera fönster.
 */
#define PORT 5555 // plus en port till för uppkopplad klient? portnummer skickas då vi syn-ack.
#define MAXLINE 1024
#define WINDOW_SIZE 5
#define TIMEOUT 1
//0.
#define RESET -1

int main(int argc, char *argv[]) {
  int s_sockfd, c_sockfd, retval, mode, state, n = 0;
  fd_set active_fd, read_fd;

  FD_ZERO(&active_fd);
  FD_ZERO(&read_fd);
  
  char *msg;
  rtp packageToSend, packageReceived;
  packageToSend.windowsize = WINDOW_SIZE;
  int sequence_number = -1;
  
  struct sockaddr_in s_addr,s_addr2, c_addr, c_addr2;

  struct timeval read_timeout;
  
  memset(&s_addr, 0, sizeof(s_addr));
  memset(&s_addr2, 0, sizeof(s_addr2));
  memset(&c_addr, 0, sizeof(c_addr));
  memset(&c_addr2, 0, sizeof(c_addr2));

  int s_addrlen = sizeof(s_addr), s_addr2len = sizeof(s_addr2),
    c_addrlen = sizeof(c_addr), c_addr2len = sizeof(c_addr2);
  
  s_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(s_sockfd < 0) {
    perror("Exit, server socket creation failure");
    exit(EXIT_FAILURE);
  }
  packageToSend.id = s_sockfd;
  
  s_addr.sin_family = AF_INET;
  s_addr.sin_addr.s_addr = INADDR_ANY; // se ip(7)
  s_addr.sin_port = htons(PORT);
  
  //	memcpy((void *)&package.id, (const void *)&s_addr, sizeof(s_addr));

  if(bind(s_sockfd, (const struct sockaddr *) &s_addr, s_addrlen) < 0){
    perror("Exit, server socket address binding error");
    exit(EXIT_FAILURE);
  }

  mode = MODE_AWAIT_CONNECT;
  state = STATE_LISTEN;
  switch(mode) {
  default :
    printf("Undefined case: mode. Exiting.\n");
    exit(EXIT_FAILURE);
    
  case MODE_AWAIT_CONNECT :
    printf("MODE: MODE_AWAIT_CONNECT.\n"); // Antar alla inkommande meddelanden är syn
    if(state == RESET)
      state = STATE_LISTEN;
    switch(state) {
    default :
      printf("Undefined case: unconnected state . Exiting.\n");
      exit(EXIT_FAILURE);

      
    case STATE_LISTEN :
      printf("STATE: listening for syn.\n");
      
      retval = recv_rtp(s_sockfd, &packageReceived, &c_addr);
      if(retval < 0) {
	printf("retval = %d. Server, listening for connection: received incorrect checksum.\n", retval);
      }
      //      else {
      printf("Server, listening for connection: received %d bytes from %s.\n", retval, inet_ntoa(c_addr.sin_addr));
      printf("Msg: %s\n", packageReceived.data);
      memset(&packageReceived, '\0', MAXLINE);

      c_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      if(c_sockfd < 0) {
	perror("Server, creating client socket");
	exit(EXIT_FAILURE);
      }

      msg = "syn-ack";
      printf("Sending: %s\n", msg);
	
      packageToSend.flags = FLAG_SYN_ACK;
      packageToSend.id = s_sockfd;
      //	memcpy((void *)&package.id, (const void *)&s_addr, sizeof(s_addr));
      packageToSend.seq = sequence_number + 1;
      memcpy(packageToSend.data, msg, strnlen(msg, MAX_DATA_LEN));
      packageToSend.windowsize = WINDOW_SIZE;

      send_rtp(c_sockfd, &packageToSend, &c_addr);
      state = STATE_AWAIT_ACK;
      //      }


    case STATE_AWAIT_ACK :
      printf("STATE: waiting for syn-ack-ack.\n");
      FD_ZERO(&active_fd);
      FD_ZERO(&read_fd);
      FD_SET(c_sockfd, &active_fd);
      read_fd = active_fd;
      
      switch(select(c_sockfd + 1, &read_fd, NULL, NULL, NULL)) {
      case -1:
	perror("Server, select c_sockfd");
	exit(EXIT_FAILURE);
      case 0:
	printf("Server, select c_sockfd: nothing to read. Returning to listen state.\n");
	close(c_sockfd);
	state = STATE_LISTEN;
	FD_ZERO(&active_fd);
	break;
      default:
	retval = recv_rtp(c_sockfd, &packageReceived, &c_addr2);
	if(retval < 0) {
	  printf("Server, listening for syn-ack-ack: incorrect checksum.");
	}
	//	else {
	printf("Server, listening for syn-ack-ack: received %d bytes from %s.\n", retval, inet_ntoa(c_addr2.sin_addr));
	printf("Msg: %s\n", packageReceived.data);
	memset(&packageReceived, '\0', MAXLINE);


	retval = recv_rtp(c_sockfd, &packageReceived, &c_addr2);
	if (retval < 0) {
	  perror("Something: incorrect checksum received.");
	}
	printf("Msg flag: %d", packageReceived.flags);
	printf("Msg data: %s", packageReceived.data);
	state = STATE_CONNECTED;
	
	//	}
      
      case STATE_CONNECTED :
	printf("STATE: syn-ack-ack received.\n");
	mode = MODE_CONNECTED;
	state = RESET;
     
      }
      if(mode != MODE_CONNECTED)
	break;


      
    case MODE_CONNECTED :
      printf("MODE: MODE_CONNECTED.\n");
    
      while(mode == MODE_CONNECTED) {
	read_fd = active_fd;


	switch(select(c_sockfd + 1, &read_fd, NULL, NULL, NULL)) {
	case -1:
	  perror("MODE_CONNECTED, select c_sockfd");
	  exit(EXIT_FAILURE);
	
	case 0:
	  printf("MODE_CONNECTED, select c_sockfd: nothing to read.\n");
	  break;
	
	default:
	  retval = recv_rtp(c_sockfd, &packageReceived, &c_addr2);
	  if(retval < 0) {
	    printf("MODE_CONNECTED, listening for message: incorrect checksum.");
	  }
	  //	  else {
	  printf("MODE_CONNECTED, listening for  message: received %d bytes from %s.\n", retval, inet_ntoa(c_addr2.sin_addr));
	  printf("Msg: %s\n", packageReceived.data);
	  memset(&packageReceived.data, '\0', MAX_DATA_LEN);
	  if (packageReceived.flags == FLAG_FIN) {
	    printf("MODE_CONNECTED, received FIN flag.\n");
	    mode = MODE_TEARDOWN;
	    state = STATE_CONNECTED;
	    break;
	  }
	  //	  }
	}

	msg = "\0";
	packageToSend.flags = FLAG_ACK;

	packageToSend.seq = sequence_number++;
	memcpy(packageToSend.data, msg, strnlen(msg, MAX_DATA_LEN));
	packageToSend.windowsize = WINDOW_SIZE;

	send_rtp(c_sockfd, &packageToSend, &c_addr);
	
	n++;
	break;
      }
    
      //    break;
      
    case MODE_TEARDOWN : // receiver version
      printf("MODE: MODE_TEARDOWN\n");
      switch(state) {
      default:
	printf("MODE_TEARDOWN unknown state.");
	break;
	
      case STATE_CONNECTED:
	// FIN received. Send ack.
	packageToSend.flags = FLAG_FIN_ACK;
	packageToSend.id = s_sockfd;
	packageToSend.seq = sequence_number++;
	memset(&packageToSend.data, '\0', MAX_DATA_LEN);
	packageToSend.windowsize = WINDOW_SIZE;
	send_rtp(c_sockfd, &packageToSend, &c_addr);

	state = STATE_SHUTTING_DOWN;

      case STATE_SHUTTING_DOWN:
	//Send FIN.
	packageToSend.flags = FLAG_FIN;
	packageToSend.seq = sequence_number++;
	memset(&packageToSend.data, '\0', MAX_DATA_LEN);
	
	send_rtp(c_sockfd, &packageToSend, &c_addr);

	state = STATE_AWAIT_FIN_ACK;

      case STATE_AWAIT_FIN_ACK:
	//select med timeout
	
	read_timeout.tv_sec = TIMEOUT;
	read_timeout.tv_usec = 0;
	
	switch(select(c_sockfd + 1, &read_fd, NULL, NULL, &read_timeout)) {
	case -1:
	  perror("STATE_AWAIT_FIN_ACK");
	  break;
	case 0:
	  printf("STATE_AWAIT_FIN_ACK timeout, disconnecting.");
	  break;
	default:
	  retval = recv_rtp(c_sockfd, &packageReceived, &c_addr);
	  if(retval < 0)
	    printf("STATE_AWAIT_FIN_ACK: Checksum error.\n");
	  if(packageReceived.flags == FLAG_FIN_ACK)
	    printf("STATE_AWAIT_FIN_ACK: FIN-ACK received, disconnecting.\n");
	  break;
	}
	
	state = STATE_DISCONNECTED;
      case STATE_DISCONNECTED:
	printf("STATE_DISCONNECTED\n");
	break;
      }
    }
  }
  
  usleep(5000);
  //  close(c_sockfd);
  close(s_sockfd);
  exit(EXIT_SUCCESS);
}
