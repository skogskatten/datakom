/*************************************************
 * File: functions.c                             *
 * Name: Shared functions        				 *
 * Authors: agn, ark                             *
 * Purpose: Contain shared functions             *
 * ***********************************************/

#include "functions.h"

int makeSocket(u_int16_t port, struct sockaddr_in *name)
{
    int sock, value_one = 1;

    /* Create socket */
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(name, 0, sizeof(*name));
    
    /* Give socket a name, port nr and address. */
    name->sin_family = AF_INET;
    name->sin_port = htons(port);
    name->sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* Set socket options */
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &value_one, sizeof(value_one));
    
    return sock;
}

//serialize
//vilken storlek på int[]?
//checksum
int serialize(rtp *header, unsigned char *ser_header)
{
    ser_header[0] = header->flags;
    ser_header[1] = header->id;
    ser_header[2] = header->seq / 256; //high part
    ser_header[3] = header->seq % 256; //low part
    ser_header[4] = header->windowsize;
    ser_header[5] = header->data;
    ser_header[5 + MAX_MSG_LEN - 1] = '\n'; //makes sure to always keep null terminator
    ser_header[5 + MAX_MSG_LEN] = make_checksum(ser_header);
    
    //return success or fail? else make void
    return 0; 
}

//deserialize
//errorcheck flagga
int deserialize(rtp *header, unsigned char *ser_header)
{
    header->flags = ser_header[0];
    header->id = ser_header[1];
    header->seq = ser_header[2] * 256; //high part
    header->seq += ser_header[3];  //low part
    header->windowsize = ser_header[4];
    header->data = ser_header[5];
    header->error = check_checksum(ser_header[5 + DATA_LEN]);
    
    return rtp->error; //this good?
}

//read
//use select to not block

//write
//add error code here

void writeData(int fileDescriptor, char *message)
{
	int nOfBytes;
	printf("Writing: %s\n", message);
	nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
	printf("Message sent\n");
}

int readData(int fileDescriptor, char *message)
{
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

