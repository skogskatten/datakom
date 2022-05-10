CC = gcc
CFLAGS = -Wall
CLIBS = -lpthread
PROGRAMS = client server
ADDITIONALS = functions.c connectionreceiver.c connectionsender.c helperfunctions.c slidingreceiver.c slidingsender.c teardownreceiver.c teardownsender.c

ALL: ${PROGRAMS}

client: client.c
	${CC} ${CFLAGS} -o client client.c ${ADDITIONALS} ${CLIBS}

server: server.c
	${CC} ${CFLAGS} -o server server.c ${ADDITIONALS} ${CLIBS}

clean:
	rm -f ${PROGRAMS}

