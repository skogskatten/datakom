/*************************************************
 * File: functions.h                             *
 * Name: Shared functions        				 *
 * Authors: agn, ark                             *
 * Purpose: Contain shared functions             *
 * ***********************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define DATA_LEN 128
#define HEADER_LEN 8
#define MAX_MSG_LEN DATA_LEN - HEADER_LEN
#define PORT_NUM   5555

/* rtp struct
 * The reliable transfer protocol header.
 * Contains all info needed for the protocol.
 */
typedef struct rtp_struct
{
    unsigned char flags;      
    unsigned char id;
    unsigned int seq;
    unsigned char windowsize;
    unsigned int crc;
    unsigned char *data;
    unsigned char error;
} rtp;

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
int makeSocket(u_int16_t port, struct sockaddr_in *name);

/* ADD DESCRIPTION HERE */
int serialize(rtp *header, unsigned char *ser_header);

int deserialize(rtp *header, unsigned char *ser_header);

/* writeMessage
 * Writes the rtp struct to the file (socket) 
 * denoted by fileDescriptor.
 */
void writeMessage(int fileDescriptor, rtp *data);

/* readMessageFromClient
 * Reads data from the file (socket)
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessage(int fileDescriptor, rtp *data);

