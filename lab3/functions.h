/*************************************************
 * File: functions.h                             *
 * Name: Shared functions        				 *
 * Authors: agn, ark                             *
 * Purpose: Contain shared functions             *
 * ***********************************************/

#include <arpa/inet.h>

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


/* ERROR MODE
 * 0 for no errors
 * 1 to 100 for % errors
 */
#define ERROR_CHANCE 0

/* Data lengths
 * Defines length of package, checksum, header
 * and data arrays in bytes (i.e. UNSIGNED CHARS).
 */
#define PACKAGE_LEN 32
#define CHECKSUM_LEN 1
#define HEADER_LEN 5
#define MAX_DATA_LEN PACKAGE_LEN - HEADER_LEN - CHECKSUM_LEN

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





#define PORT 5555
#define MAXLINE 1024
#define hostNameLength 50
#define WINDOW_SIZE 5
#define RESET -1
#define SHORT_TIMEOUT 1
#define MEDIUM_TIMEOUT 5
#define LONG_TIMEOUT 10

/* Mode functions */

/* Teardown mode when started by sending FIN */
void TeardownSender(int *state, int *mode, int writeSock, int readSock, fd_set write_fd, fd_set read_fd, int *lastSeqSent, int *lastSeqReceived, rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);

/* Teardown mode when started by receiving FIN */
void TeardownReceiver(int *state, int *mode, int writeSock, int readSock, fd_set write_fd, fd_set read_fd, int *lastSeqSent, int *lastSeqReceived, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);

/* Listens for incoming data from connected client/server, send ACK if apropriate (WARNING! Contains no actual sliding.) */
void SlidingReceiver(int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr, int *teardownSenderMode);

/* Sends data to connected client/server. Contains the sliding window. */
void SlidingSender(char *msg, int *timeoutCounter, int *state, int *mode, int writeSock, int readSock, fd_set read_fd, fd_set write_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize,rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);

/* Listens for SYN packages. Loops back to the listening state if connection is not completed. Only returns if an ACK for the SYN-ACK is received. It initialises the window and socket file descriptors if a connection is established. */
void ConnectionReceiver(int *state, int *mode, int *clientSock, int serverSock, fd_set *read_fd, fd_set *write_fd, int *lastSeqReceived, int *lastSeqSent, int *windowSize, rtp *sendWindow, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);

/* Sends SYN package. Keeps sending and listening for SYN-ACK until one is received. Then sends ACK and goes to MODE_CONNECTED. */
void ConnectionSender(int *state, int *mode, int sockfd, fd_set read_fd, fd_set write_fd, int *lastSeqReceived, int *lastSeqSent, int windowSize, struct sockaddr_in *remoteAddr, struct sockaddr_in *localAddr);


/* Helper functions */

/* Sets the data of a rtp to null */
void CleanRtpData(rtp *toClean);

/* Prints out the contents of the window. */
void PrintWindow(rtp *window, int windowSize);

/* Checks if window is full, returns 1 if full, else 0.*/
int IsWindowFull(rtp *window, int windowSize);

/* Adds a package to the sliding window. Used when sending data packages. Returns -1 if no free slot is found, else 0 upon success. */
int AddToWindow(rtp *window, int windowSize, rtp toAdd);

/* Removes a package (identified by sequence number) from the window (used when ACKs are received) Returns 0 removed correctly, -1 if package was not found. */ 
int RemoveFromWindow(rtp *window, int windowSize, unsigned int seqToRemove);

/* Removes all packages with a sequence number lower than or equal to ackedSeq from the window. Returns the number of packages removed. */
int RemoveAcknowledgedFromWindow(rtp *window, int windowSize, unsigned int ackedSeq);

/* Returns rtp with given sequence number from window. Returns dummy with seq 0 if sequence number is not found. Useful when resending. */
rtp GetFromWindow(rtp *window, int windowSize, unsigned int seqToGet);

/* Resends all rtp in the window. */
int ResendWindow(rtp *window, int windowSize, int sockfd, struct sockaddr_in *remoteAddr);

/* Zeroes all seq.num. in window, effectively emptying the window. */
int ZeroWindow(rtp *window, int windowSize);

/* Uses malloc to allocate and then initialise the server window. */
rtp *AllocateWindow(int windowSize);
