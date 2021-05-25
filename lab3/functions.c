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

unsigned int makeChecksum(const rtp *header)
{
    unsigned int checksum = 0;
    unsigned char temp_ser_header[HEADER_LEN];
    
    /* temp store a ser_header */
    serialize(temp_ser_header, header);
    
    /* Sum all elements of package */
    for(int i = 0; i < (PACKAGE_LEN - CHECKSUM_LEN); i++)
    {
        checksum += temp_ser_header[i];
    }
    
    return checksum;
}

int checkChecksum(const rtp *header)
{
    if(makeChecksum == rtp->crc)
        return 0;
    else
        return -1;
}

void serialize(unsigned char *ser_header, const rtp *header)
{
    /* Add all header variables to their positions */
    ser_header[0] = header->flags;
    ser_header[1] = header->id;
    ser_header[2] = header->seq / 256; //high part
    ser_header[3] = header->seq % 256; //low part
    ser_header[4] = header->windowsize;
    /* Add data at the right position after header */
    memcpy(ser_header + HEADER_LEN, header->data, MAX_DATA_LEN);
}

int deserialize(rtp *header, const unsigned char *ser_header)
{
    header->flags = ser_header[0];
    header->id = ser_header[1];
    header->seq = ser_header[2] * 256; //high part
    header->seq += ser_header[3];      //low part
    header->windowsize = ser_header[4];
    header->data = ser_header + 5;
    header->checksum = ser_header + 6;
    
    return checkChecksum(header);
}

void send_rtp(int sockfd, const rtp* package, sockaddr_in *addr)
{
    unsigned char ser_package[PACKAGE_LEN];
    
    serialize(ser_package, package);
    makeChecksum(package);
    if(sendto(sockfd, ser_package, PACKAGE_LEN, 0,
              (const struct sockaddr *) addr, sizeof(*addr)) < 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

int recv_rtp(int sockfd, rtp* package, sockaddr_in addr*)
{  
    int nOfBytes;
    char buffer[PACKAGE_LEN];
    
    if(recvfrom(sockfd, (unsigned char*)buffer, PACKAGE_LEN, MSG_WAITALL,
             addr, sizeof(*addr)) < 0)
    {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    
    if(deserialize(package, buffer) < 0)
        return -1;
    else
        return nOfBytes;
        
    
    return nOfBytes;
}
