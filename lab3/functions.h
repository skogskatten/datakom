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

/* Data lengths
 * Defines length of package, checksum, header
 * and data arrays in bytes (i.e. UNSIGNED CHARS).
 */
#define PACKAGE_LEN 32
#define CHECKSUM_LEN 1
#define HEADER_LEN 5 + CHECKSUM_LEN
#define MAX_DATA_LEN PACKAGE_LEN - HEADER_LEN

#define PORT_NUM   5555

/* Modes and states for state machines */
#define MODE_AWAIT_CONNECT  0
#define MODE_CONNECTED      1
#define MODE_TEARDOWN       2

#define STATE_WANT_CONNECT  3
#define STATE_AWAIT_SYN_ACK 4
#define STATE_CONNECTED     5
#define STATE_AWAIT_ACK     6
#define STATE_SEND          7
#define STATE_LISTEN        8
#define STATE_SHUTTING_DOWN 9
#define STATE_AWAIT_FIN_ACK 10
#define STATE_AWAIT_FIN     11
#define STATE_TIMEOUT       12
#define STATE_DISCONNECTED  13

/* Package flags */ 
#define FLAG_DATA    0
#define FLAG_ACK     1
#define FLAG_SYN     2
#define FLAG_SYN_ACK 3
#define FLAG_FIN     4
#define FLAG_FIN_ACK 5

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
    unsigned char data[MAX_DATA_LEN];
    unsigned char crc;
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
unsigned char makeChecksum(rtp *header);

/* ADD DESCRIPTION HERE */
int checkChecksum(const rtp *header);

/* ADD DESCRIPTION HERE */
void serialize(unsigned char *ser_header, const rtp *header);

/* ADD DESCRIPTION HERE */
int deserialize(rtp *header, const unsigned char *ser_header);

void send_rtp(int sockfd, rtp* package, struct sockaddr_in *addr);

/* recv_rtp 
 * returns bytes read, if return < 0 checksum was wrong
 */
int recv_rtp(int sockfd, rtp* package, struct sockaddr_in *addr);

void print_rtp_header();

void print_rtp(rtp *package);
