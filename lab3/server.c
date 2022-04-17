#include "functions.h"
/* Att göra
 * 1. Ordna upp tillstånd. Lägg ev. till något om det saknas.
 * 2. Testa om fortfarande fungerar som server3 gjorde.
 * 3. Sekvensnummer? Lokala. ACKar innehåller sekvensnummer som data.
 * 4. Implementera checksummehantering.
 * 5. Implementera fönster.
 */
#define PORT 5555 // plus en port till för uppkopplad klient? portnummer skickas då vi syn-ack.
#define MAXLINE 1024
#define WINDOW_SIZE 5
#define TIMEOUT 1
//0.
#define RESET -1

int main(int argc, char *argv[]) {
  int s_sockfd, c_sockfd, retval, mode, state, n = 0;
  int lastSeqSent = 0, lastSeqReceived = 0, timeoutCounter = 0;
  int windowSize;
  fd_set active_fd, read_fd, write_fd;
  
  FD_ZERO(&active_fd);
  FD_ZERO(&read_fd);
  FD_ZERO(&write_fd);
  
  char *msg = "Msg from the server.";
  rtp packageToSend, packageReceived;
  rtp *window; // allocated in the connection phase.
  
  struct sockaddr_in s_addr,s_addr2, c_addr, c_addr2;
  
  struct timeval read_timeout;
  
  memset(&s_addr, 0, sizeof(s_addr));
  memset(&s_addr2, 0, sizeof(s_addr2));
  memset(&c_addr, 0, sizeof(c_addr));
  memset(&c_addr2, 0, sizeof(c_addr2));
  
  int s_addrlen = sizeof(s_addr);
  
  s_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(s_sockfd < 0) {
    perror("Exit, server socket creation failure");
    exit(EXIT_FAILURE);
  }

  s_addr.sin_family = AF_INET;
  s_addr.sin_addr.s_addr = INADDR_ANY; // se ip(7)
  s_addr.sin_port = htons(PORT);

  //  inet_ntoa(c_addr.sin_addr)
  memcpy((void *)&packageToSend.id, (const void *)inet_ntoa(s_addr.sin_addr), strlen(inet_ntoa(s_addr.sin_addr)));
  
  if(bind(s_sockfd, (const struct sockaddr *) &s_addr, s_addrlen) < 0){
    perror("Exit, server socket address binding error");
    exit(EXIT_FAILURE);
  }
  
  mode = MODE_AWAIT_CONNECT;
  state = STATE_LISTEN;

  /* Here is the main mode/state loop. It will loop until a connection has been both established and shut down. */
  while(state != STATE_DISCONNECTED) {
    switch(mode) {
    default :
      printf("Undefined case: mode. Exiting.\n");
      exit(EXIT_FAILURE);
    
    case MODE_AWAIT_CONNECT :
      //connectionreceiver
    
      break;
      
    case MODE_CONNECTED :
      printf("MODE: MODE_CONNECTED.\n");
    
      while(mode == MODE_CONNECTED) {
      
	SlidingReceiver(&timeoutCounter, &state, &mode, c_sockfd, c_sockfd, read_fd, &lastSeqReceived, windowSize, window,  sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);
	if (mode == MODE_CONNECTED)
	  SlidingSender(msg, &timeoutCounter, &state, &mode, c_sockfd, c_sockfd, active_fd, active_fd, &lastSeqReceived, &lastSeqSent, windowSize, window, &c_addr, &s_addr);
	
      }
    
      break;
      
    case MODE_TEARDOWN : // receiver version
      printf("MODE: MODE_TEARDOWN\n");
      
      TeardownReceiver(&state, &mode, c_sockfd, c_sockfd, active_fd, active_fd, &lastSeqSent, &lastSeqReceived, &c_addr, &s_addr);
      
      break;
    }
  }

  
  usleep(5000);
  //  close(c_sockfd);
  close(s_sockfd);
  free(window);
  exit(EXIT_SUCCESS);
}
