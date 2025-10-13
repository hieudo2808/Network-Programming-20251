CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.c FileHandler/fileprocess.c LinkedList/account.c LinkedList/session.c Validation/validation.c ServerHandler/serverHandler.c
	$(CC) $(CFLAGS) -o server server.c FileHandler/fileprocess.c LinkedList/account.c LinkedList/session.c Validation/validation.c ServerHandler/serverHandler.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f *.o server client
	rm -f FileHandler/*.o LinkedList/*.o Validation/*.o ServerHandler/*.o

.PHONY: all clean
