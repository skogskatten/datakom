/* File: server.c
 * Name: Server program
 * Authors: ########, ###############
 * Description: Trying out socket communication between processes using TCP.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT 5555
#define MAXMSG 512


/* makeSocket
 * Creates and names a socket in the Internet
 * name-space. The socket created exists
 * on the machine from which the function is 
 * called. Instead of finding and using the
 * machine's Internet address, the function
 * specifies INADDR_ANY as the host address;
 * the system replaces that with the machine's
 * actual address.
 */
int makeSocket(unsigned short int port) {
	int sock;
	struct sockaddr_in name;
	int valueOne = 1;

	/* Create a socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
	/* Give the socket a name. */
	/* Socket address format set to AF_INET for Internet use. */
	name.sin_family = AF_INET;
	/* Set port number. The function htons converts from host byte order to network byte order.*/
	name.sin_port = htons(port);
	/* Set the Internet address of the host the function is called from. */
	/* The function htonl converts INADDR_ANY from host byte order to network byte order. */
	/* (htonl does the same thing as htons but the former converts a long integer whereas
	 * htons converts a short.) 
	 */
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Set sock option to allow a server to reconnect before 120s timeout. */
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&valueOne, sizeof(valueOne)) < 0) {
		perror("Could not set socket filter\n");
		exit(EXIT_FAILURE);
	}

	/* Assign an address to the socket by calling bind. */
	if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		perror("Could not bind a name to the socket\n");
		exit(EXIT_FAILURE);
	}

	return(sock);
}

/* writeMessage
 * Writes the string message to the file (socket) 
 * denoted by fileDescriptor.
 */
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

/* readMessageFromClient
 * Reads and prints data read from the file (socket
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessageFromClient(int fileDescriptor) {
	char buffer[MAXMSG];
	int nOfBytes;

	nOfBytes = read(fileDescriptor, buffer, MAXMSG);
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

int main(int argc, char *argv[]) {
	int sock;
	int clientSocket;
	int i, j;
	fd_set activeFdSet, readFdSet; /* Used by select */
	struct sockaddr_in clientName;
	socklen_t size;
	char response[20] = "I hear ya";
	char broadcast[70];
	char banned_ip[20];
	struct sockaddr_in buffer;

	/* Asks for an ipv4 address to ban. If none is entered, no ip is banned. */
	printf("Enter an ip(v4) to ban (leave empty if none): ");
	fgets(banned_ip, 20, stdin);
	banned_ip[strlen(banned_ip) - 1] = '\0';
	if(banned_ip[0] != '\0')
		printf("Banned ip: %s\n", banned_ip);
	else
		printf("No ip was banned.\n");

	/* Create a socket and set it up to accept connections */
	sock = makeSocket(PORT);
	/* Listen for connection requests from clients */
	if(listen(sock,1) < 0) {
		perror("Could not listen for connections\n");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets */
	FD_ZERO(&activeFdSet);
	FD_SET(sock, &activeFdSet);

	/* Main program loop */
	printf("\n[waiting for connections...]\n");
	while(1) {
		/* Block until input arrives on one or more active sockets
		   FD_SETSIZE is a constant with value = 1024 */
		readFdSet = activeFdSet;

		/* blocks until there is a socket with something to read */
		if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
			perror("Select failed\n");
			exit(EXIT_FAILURE);
		}
		/* Service all the sockets with input pending */
		for(i = 0; i < FD_SETSIZE; ++i) 
			if(FD_ISSET(i, &readFdSet)) { // Kollar om i finns i anslutet set
				if(i == sock) { // Om i är vår lyssnarsocket
					/* Connection request on original socket */
					size = sizeof(struct sockaddr_in);

					read(sock, (void *)&buffer, size+1);
					printf("Read %s from socket.\n", inet_ntoa(buffer.sin_addr));

					/* Accept the connection request from a client. */
					clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size); 
					if(clientSocket < 0) {
						perror("Could not accept connection\n");
						exit(EXIT_FAILURE);
					}

					/* blacklist check  */
					if(banned_ip[0] != '\0') {
						printf("Comparing blacklisted ip \"%s\" to connecting ip \"%s\"\n", banned_ip, inet_ntoa(clientName.sin_addr));
						if(strcmp( inet_ntoa(clientName.sin_addr), banned_ip) == 0 ) {
							writeMessage(clientSocket, "Connection refused; ip blacklisted.\n");
							continue; /* if ip is banned, continue without broadcasting or adding to socket fd set */
						}
					}

					printf("Server: Connect from client %s, port %d\n",
							inet_ntoa(clientName.sin_addr),
							ntohs(clientName.sin_port));

					/* broadcast loop */
					for(j = 0; j < FD_SETSIZE; ++j) {
						if(FD_ISSET(j, &activeFdSet)) { /* Checks if j is in connected set */
							printf("Attempting broadcast\n");
							if(j != sock) {
								printf("%d. ", j);
								sprintf(broadcast, "%s:%d\n",
										inet_ntoa(clientName.sin_addr),
										ntohs(clientName.sin_port));
								printf("Sending message: %s", broadcast);
								writeMessage(j, broadcast);
							}
						}
					}

					FD_SET(clientSocket, &activeFdSet);
				}
				else { // annars lyssnar efter meddelande
					/* Data arriving on an already connected socket */
					if(readMessageFromClient(i) < 0) {
						close(i);
						FD_CLR(i, &activeFdSet);
					}
					else {
						writeMessage(i, response);
					}
				}
			}
	}
}

