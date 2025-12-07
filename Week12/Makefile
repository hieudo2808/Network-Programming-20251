CC = gcc
CFLAGS = -Wall -g

SERVER = server
CLIENT = client

SERVER_SRC = server.c account.c session.c auth_log.c
CLIENT_SRC = client.c

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) -lpthread

$(CLIENT):
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC) -lpthread

clean:
	rm -f $(SERVER) $(CLIENT)

.PHONY: all clean
