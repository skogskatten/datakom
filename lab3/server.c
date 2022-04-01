#include "functions.h"

#define PORT 5555 // plus en port till för uppkopplad klient? portnummer skickas då vi syn-ack.
#define MAXLINE 1024
#define WINDOW_SIZE 5
//0.
#define RESET -1

int main(int argc, char *argv[]) {
  int s_sockfd, c_sockfd, retval, mode, state, n = 0;
  fd_set active_fd, read_fd;
  char buffer[MAXLINE];
  char *msg;
  rtp package;
  unsigned char serialized_package;
  int sequence_number = -1;
  
  struct sockaddr_in s_addr,s_addr2, c_addr, c_addr2;

  FD_ZERO(&active_fd);
  FD_ZERO(&read_fd);
  memset(&buffer, 0, MAXLINE);
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
  
  s_addr.sin_family = AF_INET;
  s_addr.sin_addr.s_addr = INADDR_ANY; // se ip(7)
  s_addr.sin_port = htons(PORT);
  
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
      
      retval = recv_rtp(s_sockfd, &package, &c_addr);
      if(retval == -1) {
	perror("Server, listening for connection: received incorrect checksum.");
	break;
      }
      else {
	printf("Server, listening for connection: received %d bytes from %s.\n", retval, inet_ntoa(c_addr.sin_addr));
	printf("Msg: %s\n", buffer);
	memset(&buffer, '\0', MAXLINE);

	c_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(c_sockfd < 0) {
	  perror("Server, creating client socket");
	  exit(EXIT_FAILURE);
	}

	msg = "syn-ack";
	printf("Sending: %s\n", msg);
	
	package.flags = FLAG_SYN_ACK;
	package.id = s_sockfd;
	//	memcpy((void *)&package.id, (const void *)&s_addr, sizeof(s_addr));
	package.seq = sequence_number + 1;
	memcpy(package.data, msg, strnlen(msg, MAX_DATA_LEN));
	package.windowsize = WINDOW_SIZE;

	send_rtp(c_sockfd, &package, &c_addr);
	state = STATE_LISTEN_WAIT_SYNACKACK;
      }


    case STATE_LISTEN_WAIT_SYNACKACK :
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
	retval = recv_rtp(c_sockfd, &package, &c_addr2);
	if(retval < 0) {
	  perror("Server, listening for syn-ack-ack");
	  exit(EXIT_FAILURE);
	}
	else {
	  printf("Server, listening for syn-ack-ack: received %d bytes from %s.\n", retval, inet_ntoa(c_addr2.sin_addr));
	  printf("Msg: %s\n", buffer);
	  memset(&buffer, '\0', MAXLINE);


	  retval = recv_rtp(c_sockfd, &package, &c_addr2);
	  state = STATE_LISTEN_ACK_RECEIVED;
	
	}
      
      case STATE_LISTEN_ACK_RECEIVED :
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
	  perror("Server, select c_sockfd");
	  exit(EXIT_FAILURE);
	
	case 0:
	  printf("Server, select c_sockfd: nothing to read.\n");
	  break;
	
	default:
	  retval = recv_rtp(c_sockfd, &package, &c_addr2);
	  if(retval < 0) {
	    perror("Server, listening for 1st message");
	  }
	  else {
	    printf("Server, listening for 1st message: received %d bytes from %s.\n", retval, inet_ntoa(c_addr2.sin_addr));
	    printf("Msg: %s\n", buffer);
	    memset(&buffer, '\0', MAXLINE);
	  }
	}

	msg = "\0";
	package.flags = FLAG_ACK;
	package.id = s_sockfd;
	//	memcpy((void *)&package.id, (const void *)&s_addr, sizeof(s_addr));
	package.seq = sequence_number + 1;
	memcpy(package.data, msg, strnlen(msg, MAX_DATA_LEN));
	package.windowsize = WINDOW_SIZE;

	send_rtp(c_sockfd, &package, &c_addr);
	
	n++;
      }
    
      //    break;
  
    case TEARDOWN :
      printf("MODE: TEARDOWN\n");
    }
  }
  
  usleep(5000);
  //  close(c_sockfd);
  close(s_sockfd);
  exit(EXIT_SUCCESS);
}
