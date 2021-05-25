/*************************************************
 * File: server.c                                *
 * Name: Server program with UDP                 *
 * Authors: agn, ark                             *
 * Purpose:                                      *
 * ***********************************************/

#include "functions.c"

int main(void)
{
    int sock, state, i, j;
    struct sockaddr_in server_addr;
    fd_set sock_fd_set, read_fd_set, write_fd_set;

    /* Create and initialize socket */
    printf("Initializing server.\n");
    sock = makeSocket(PORT_NUM, &server_addr);

    /* Assign address to socket */
    if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* Initialize sock_fd_set to a null (empty) set */
    FD_ZERO(&sock_fd_set);
    /* Add sock to the sock_fd_set */
    FD_SET(sock, &sock_fd_set);

    /* Main program loop */
    printf("Initialized, waiting for connections.\n");
    while (1)
    {
        int mode, state, event;
        read_fd_set = sock_fd_set;
        write_fd_set = sock_fd_set;

        /* Select selects the sockets ready for input or output.
         * Not clear if we need write_fd_set here, or at all */
        if (select(FD_SETSIZE, &read_fd_set, &write_fd_set, NULL, NULL, NULL) < 0)
        {
            perror("Select failed\n");
            exit(EXIT_FAILURE);
        }

        /* nedan följer pseudokod med förslag på tillståndsflöde.
         * Fungerar endast för 1 klient? Kanske? Varje ny klient får,
         * efter connect, en egen socket där meddelanden från klienten
         * tas emot/skickas(?), vilket gör AWAIT_CONNECT inaktuellt för
         * de andra socketarna, samtidig som socket 0, serverns "publika"
         * socket aldrig lämnar uppkoppling. Borde funka bra med en if-sats
         * som slänger ut alla klientbrevlådor ur AWAIT_CONNECT
         */
        
        for(i = 0; i < FD_SETSIZE; i++)
        {
            if(FD_ISSET(i, read_fd_set))
            {
                switch(mode)
                {
                case (MODE_AWAIT_CONNECT):
                    //Only sock should ever use this mode
                    if (curr != sock)
                    {
                        mode = CONNECTED;
                        
                    }
                    break;
                        
                    switch (state)
                    {
                    //Listening
                    case(STATE_LISTEN):
                        // EVENT: receives SYN / send SYN-ACK -> state = Wait
                        break;

                    case(STATE_AWAIT_SYN_ACK):
                        switch(event)
                        {
                        case(STATE_AWAIT_ACK):
                            // ACK / Connect -> state = Connected
                            break;

                        case(STATE_TIMEOUT):
                            // Timeout / - -> state = LISTENING
                            break;
                        }
                        break;

                    case(STATE_CONNECTED):
                        if (curr == sock)
                            state = LISTENING;
                        else {
                            mode = CONNECTED;
                            state = RECIEVE;
                        }
                        break;
                    }
                    break;

                case(MODE_CONNECTED):
                    switch(state)
                    {
                    case(STATE_LISTEN): //wait
                        switch (event)
                        {
                        case (PACKAGE_EXPECTED):
                            //send ack
                            break;
                        case (PACKAGE_WRONG):
                            //send prev ack
                            break;
                        case (PACKAGE_FIN):
                            // - / Send FIN-ACK -> mode =  TEARDOWN, state = SHUTTING_DOWN
                            break;
                        }
                    }
                    break;

                case(TEARDOWN):
                    /* Server has received a FIN */
                    switch(state)
                    {
                    case(STATE_SHUTTING_DOWN):
                        // Shutting down, preparing to close connection
                        // Do the needful / Send FIN -> state WAITING_FOR_FIN-ACK
                        break;

                    case(STATE_AWAIT_FIN_ACK):
                        switch(event)
                        {
                        case (FIN - ACK):
                            // - / Disconnect -> state = disconnected
                            break;

                        case (TIMEOUT):
                            // - / Disconnect -> state = disconnected
                            break;
                        default:
                            // Do nothing
                            break;
                        }
                        break;
                        // DISCONNECTED
                    }
                    default;
                    //no op
                } //switch(mode)
            } //if FD_ISSET
        } //for FD_SETSIZE
    } //while
    //  unlink((struct sockaddr *) &server_addr);
    return 0;
}
