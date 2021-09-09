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

unsigned char makeChecksum(rtp *header)
{
    unsigned int checksum = 0;
    unsigned char temp_ser_header[PACKAGE_LEN - CHECKSUM_LEN];
    
    /* Temp store a ser_header */
    serialize(temp_ser_header, header);
    
    /* Sum all elements of package */
    for(int i = 0; i < PACKAGE_LEN - CHECKSUM_LEN; i++)
    {
        checksum += temp_ser_header[i]; 
    }
    
    checksum = (unsigned char)(checksum % 256);
    
    header->crc = checksum;
    
    return checksum;
}

int checkChecksum(const rtp *header)
{
    unsigned int checksum = 0;
    unsigned char temp_ser_header[PACKAGE_LEN - CHECKSUM_LEN];
    
    /* Temp store a ser_header */
    serialize(temp_ser_header, header);
    
    /* Sum all elements of package */
    for(int i = 0; i < PACKAGE_LEN - CHECKSUM_LEN; i++)
    {
        checksum += temp_ser_header[i]; 
    }
    
    checksum = (unsigned char)(checksum % 256);
    
    if(checksum == header->crc)
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
    
    /* Add data string after header */
    memcpy(ser_header + 5, header->data, MAX_DATA_LEN);
}

int deserialize(rtp *header, const unsigned char *ser_header)
{
    header->flags = ser_header[0];
    header->id = ser_header[1];
    header->seq = ser_header[2] * 256; //high part
    header->seq += ser_header[3];      //low part
    header->windowsize = ser_header[4];
    memcpy(header->data, ser_header + HEADER_LEN, MAX_DATA_LEN);
    header->crc = ser_header[31];
    
    return checkChecksum(header);
}

void send_rtp(int sockfd, rtp *package, struct sockaddr_in *addr)
{
    unsigned char crc;
    unsigned char ser_package[PACKAGE_LEN];
    
    serialize(ser_package, package);
    
    /* Checksum is added to ser_package */
    crc = makeChecksum(package);
    ser_package[PACKAGE_LEN - CHECKSUM_LEN] = crc;
    
    /* Generate errors into ser_package[] here */
    if(ERROR_CHANCE > rand() % 100)
    {
        int rand_mod = rand() % 5;
        /* Error mode
        * default - Lose package
        * 0 - Change header
        * 1 - Change data
        * 2 - Chance crc */
        
        printf("[ERR!] ");
        switch(rand_mod)
        {
            case 0:
                printf("Header modified\n");
                for(int i=0; i < HEADER_LEN; i++)
                {
                    ser_package[i] = (unsigned char)(rand()%256);
                }
                break;
            case 1:
                printf("Data modified\n");
                for(int i=HEADER_LEN; i < HEADER_LEN + 12; i++)
                {
                    ser_package[i] = (unsigned char)(32 + rand() % 95);
                }
                ser_package[HEADER_LEN + 12] = '\0';
                break;
            case 2:
                printf("Checksum modified\n");
                ser_package[PACKAGE_LEN-CHECKSUM_LEN] = (unsigned char)(rand()%256);
                break;
            default:
                printf("Package lost\n");
                return;
                break;
        }
    }
    /* ###### Error generation end ###### */
    
    if(sendto(sockfd, ser_package, PACKAGE_LEN, 0,
              (const struct sockaddr *) addr, sizeof(*addr)) < 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
}

int recv_rtp(int sockfd, rtp *package, struct sockaddr_in *addr)
{  
    int nOfBytes = 0;
    unsigned int len = sizeof(addr);
    unsigned char buffer[PACKAGE_LEN];
    
    nOfBytes = recvfrom(sockfd, (unsigned char*)buffer, PACKAGE_LEN, MSG_WAITALL,
              (struct sockaddr*) addr, &len);
    
    if(nOfBytes < 0)
    {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    
    /* Check for errors */
    if(deserialize(package, buffer) < 0)
        return -1;
    else
        return nOfBytes;
}

void print_rtp_header()
{
    printf("ACTION\tFLAGS\tID\tSEQ\tWIN\tDATA\t\tCRC\n");
}

void print_rtp(rtp *package)
{
    printf("\t%02X\t%02X\t%02X\t%02X\t%s\t%02X\n",
           package->flags, package->id, package->seq,
           package->windowsize, package->data, package->crc);
}
