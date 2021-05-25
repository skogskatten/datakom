CC = gcc
CFLAGS = -Wall -g
CLIBS = -lpthread
PROGRAMS = client server

ALL: ${PROGRAMS}

client: client.c
	${CC} ${CFLAGS} -o client client.c ${CLIBS}

server: server.c
	${CC} ${CFLAGS} -o server server.c ${CLIBS}

clean:
	rm -f ${PROGRAMS}
