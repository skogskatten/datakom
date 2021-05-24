/*************************************************
 * File: functions.c                             *
 * Name: Shared functions        				 *
 * Authors: agn, ark                             *
 * Purpose: Contain shared functions             *
 * ***********************************************/

#include <functions.h>

int makeSocket(u_int16_t port)
{
    int sock, value_one = 1;
    struct sockaddr_in name;

    /* Create socket */
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));
    
    /* Give socket a name, port nr and address. */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Assign address to socket */
    if(bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void writeData(int fileDescriptor, char *message) {
	int nOfBytes;
	printf("Writing: %s\n", message);
	nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
	printf("Message sent\n");
}

int readData(int fileDescriptor, char *message) {
	int nOfBytes;

	nOfBytes = read(fileDescriptor, message, MAX_MSG_LEN);
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
	        return(0);
}

