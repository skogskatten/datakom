/*************************************************
 * File: functions.c                             *
 * Name: Shared functions        				 *
 * Authors: agn, ark                             *
 * Purpose: Contain shared functions             *
 * ***********************************************/

#define MAXMSG 512
#define PORT   5555

int makeSocket(u_int16_t port)
{
    int sock, value_one = 1;
    struct sockaddr_in name;

    /* Create socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Give socket a name, port nr and address. */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Set sock opt */
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
    (void *)&value_one, sizeof(value_one)) < 0)
    {
        perror("filter");
        exit(EXIT_FAILURE);

    }
    
    /* Assign address to socket */
    if(bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void writeMessage(int fileDescriptor, char *message) {
	int nOfBytes;
	printf("Writing: %s\n", message);
	nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
	printf("Message sent\n");
}

int readMessage(int fileDescriptor) {
      char buffer[MAX_MESSAGE_LENGTH];
      int nOfBytes;
  
      nOfBytes = read(fileDescriptor, buffer, MAX_MESSAGE_LENGTH);
      if(nOfBytes < 0) {
          perror("Could not read data from client\n");
          exit(EXIT_FAILURE);
      }
      else
          if(nOfBytes == 0)
              /* End of file */
              return(-1);
          else
              /* Data read */
              printf(">Incoming message: %s\n",  buffer);
      return(0);
}

