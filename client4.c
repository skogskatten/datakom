/* #include<stdio.h> */
/* #include<stdlib.h> */
/* #include<unistd.h> */
/* #include<string.h> */
/* #include<sys/time.h> */
/* #include<sys/types.h> */
/* #include<sys/socket.h> // struct sockaddr */
/* #include<arpa/inet.h> */
/* #include<netinet/in.h> // För struct sockaddr_in */
/* #include<netdb.h> */
#include "functions.h"

#define PORT 5555
#define MAXLINE 1024
#define hostNameLength 50

#define RESET -1

#define UNCONNECTED 0
#define CONNECTED 1
#define TEARDOWN 2

#define WANTS_TO_CONNECT 0
#define WAIT_FOR_SYNACK 1
#define SYNACK_RECEIVED 2

#define FIN_SENT_WAIT_ACK 0
#define ACK_RECVD_WAIT_FIN 1
#define FIN_RECVD_WAIT_ACK 2
#define TIMEOUT 3
#define DISCONNECTED 4

int main(int argc, char *argv[]) {
  int sockfd, retval, mode, state, n;
  char buffer[MAXLINE];
  char hostName[hostNameLength];
  char *msg;
  struct sockaddr_in s_addr,c_addr;
  fd_set read_fd, write_fd, active_fd;
  FD_ZERO(&read_fd);
  FD_ZERO(&write_fd);
  FD_ZERO(&active_fd);

  struct timeval read_timeout;
  read_timeout.tv_sec = 1;
  read_timeout.tv_usec = 0;
  
  
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

  /* if(setsockopt(sockfd, SOL_SOCKET, SO_RECVTIMEO, (const void *)&read_timeout, sizeof(read_timeout)) < 0) { */
  /*   perror("Client, setting sockopt"); */
  /*   exit(EXIT_FAILURE); */
  /* } */
  
  int s_addrlen = sizeof(s_addr), c_addrlen = sizeof(c_addr);
  
  memset(&buffer, 0, MAXLINE);
  memset(&s_addr, 0, s_addrlen);
  
  struct hostent *hostInfo; /* Contains info about the host */
  hostInfo = gethostbyname(hostName);
  if(hostInfo == NULL) {
    fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
    exit(EXIT_FAILURE);
  }
  
  s_addr.sin_addr = *(struct in_addr *)hostInfo->h_addr;
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(PORT);

  mode = UNCONNECTED;
  state = WANTS_TO_CONNECT;
  n = 0;
  switch(mode) {
  default :
    printf("Undefined case: mode . Exiting.\n");
    exit(EXIT_FAILURE);

  case UNCONNECTED :
    printf("MODE: unconnected.\n");

    switch(state) {
    default :
      printf("Undefined case: state . Exiting.\n");
      exit(EXIT_FAILURE);
      
    case WANTS_TO_CONNECT :
      printf("STATE: wants to connect.\n");
      msg = "syn";
      if(sendto(sockfd, (const void *)msg, strlen(msg), 0, (const struct sockaddr *)&s_addr, s_addrlen) < 0){
	perror("Sendto");
	exit(EXIT_FAILURE);
      }
      printf("Msg sent.\n");
      state = WAIT_FOR_SYNACK;
      
    case WAIT_FOR_SYNACK :
      printf("STATE: wait for syn-ack.\n");
      //lägg till timeout, select med timeval
      usleep(5000);
      if(retval = recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, &s_addrlen) < 0) {
	perror("Client, receiving syn-ack");
	exit(EXIT_FAILURE);
      }
      printf("Client, listening for syn-ack: received %d bytes from %s.\n", retval, inet_ntoa(s_addr.sin_addr));
      printf("Msg: %s\n", buffer);
      memset(&buffer, '\0', MAXLINE);
      state = SYNACK_RECEIVED;

    case SYNACK_RECEIVED :
      printf("STATE: syn-ack received.\n");
      /* går direkt från att skicka syn-ack-ack till att koppla upp, men om syn-ack-acken försvinner ansluter klienten men servern stänger buffern. */
      /* lös ovan med att koppla ned om sändning inte fungerar sedan, timeout i connected */
      msg = "syn-ack-ack";
      if(sendto(sockfd, (void *)msg, strlen(msg), 0,(struct sockaddr *)&s_addr, s_addrlen) < 0) {
	perror("Client, sending syn-ack-ack");
	exit(EXIT_FAILURE);
      }
      mode = CONNECTED;
    }
    
  case CONNECTED :
    printf("MODE: CONNECTED.\n");
    
    msg = "Select nr.";
    if(sendto(sockfd, (void *)msg, strlen(msg), 0,(struct sockaddr *)&s_addr, s_addrlen) < 0) {
      perror("Client, sending first select msg");
      exit(EXIT_FAILURE);
    } //Lägg till timeout ifall syn-ack-ack kommer bort
    if(retval = recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, &s_addrlen) < 0) {
      perror("Client, receiving msg ack");
      exit(EXIT_FAILURE);
    }
    //    printf("Received msg %d.\n",(int)buffer); DEBUG
    printf("Received msg %s.\n", buffer);
    memset(&buffer, '\0', MAXLINE);


    /* Just nu faller den igenom alla TIMEOUT. behöver while loop alt. ex if-else för att skippa fall då gått till timeout */  
  case TEARDOWN :
    printf("MODE: TEARDOWN.\n");

    msg = "FIN";
    if(sendto(sockfd, (void *)msg, strlen(msg), 0,(struct sockaddr *)&s_addr, s_addrlen) < 0) {
      perror("Client, sending first select msg");
      exit(EXIT_FAILURE);
    }

    state = FIN_SENT_WAIT_ACK;
    switch(state) {
    case FIN_SENT_WAIT_ACK :
      printf("STATE: FIN sent, awaiting ack.\n");
      read_fd = active_fd;
      retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
      switch(retval) {
      case -1 :
	perror("Client, select");
	exit(EXIT_FAILURE);
      case 0 :
	printf("Timeout, going to timeout state.\n");
	state = TIMEOUT;
	break;
      default :
	if(recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, &s_addrlen) < 0) {
	  perror("Client, receiving fin-ack");
	  exit(EXIT_FAILURE);
	}
	printf("Received fin-ack.\n");
	state = ACK_RECVD_WAIT_FIN;
      }

    case ACK_RECVD_WAIT_FIN :
      printf("STATE: FIN-ACK received, waiting for FIN.\n");
      read_fd = active_fd;
      retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
      switch(retval) {
      case -1 :
	perror("Client, select");
	exit(EXIT_FAILURE);
      case 0 :
	printf("Timeout, going to timeout state.\n");
	state = TIMEOUT;
	break;
      default :
	if(recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, &s_addrlen) < 0) {
	  perror("Client, receiving fin");
	  exit(EXIT_FAILURE);
	}
	printf("Received fin.\n");
	state = FIN_RECVD_WAIT_ACK;
      }
      
    case FIN_RECVD_WAIT_ACK :
      printf("STATE: FIN sent and received, waiting for FIN-ACK.\n");
      read_fd = active_fd;
      retval = select(sockfd + 1, &read_fd, NULL, NULL, &read_timeout);
      switch(retval) {
      case -1 :
	perror("Client, select");
	exit(EXIT_FAILURE);
      case 0 :
	printf("Timeout, going to timeout state.\n");
	state = TIMEOUT;
	break;
      default :
	if(recvfrom(sockfd, (void *)buffer, MAXLINE, 0, (struct sockaddr *)&s_addr, &s_addrlen) < 0) {
	  perror("Client, receiving fin-ack");
	  exit(EXIT_FAILURE);
	}
	printf("Received fin-ack.\n");
	state = TIMEOUT;
      }
    case TIMEOUT :
      usleep(500000);
      close(sockfd);
    }
  }

  usleep(50000);
  exit(EXIT_SUCCESS);

}
