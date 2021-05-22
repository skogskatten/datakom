int readMessageFromClient(int fileDescriptor) {
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
