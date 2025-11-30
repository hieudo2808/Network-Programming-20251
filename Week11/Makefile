CC = gcc

SERVER = server
CLIENT = client

SERVER_SRC = server.c account.c session.c auth_log.c
CLIENT_SRC = client.c

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC)

$(CLIENT):
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f $(SERVER) $(CLIENT)

.PHONY: all clean
