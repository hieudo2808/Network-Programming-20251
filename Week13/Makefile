CC = gcc

# Common modules
COMMON_SRCS = account.c session.c handler.c
HEADERS = account.h session.h handler.h

# Targets
all: server_select server_poll client

server_select: server_select.c $(COMMON_SRCS) $(HEADERS)
	$(CC) -o server_select server_select.c $(COMMON_SRCS)

server_poll: server_poll.c $(COMMON_SRCS) $(HEADERS)
	$(CC) -o server_poll server_poll.c $(COMMON_SRCS)

client: client.c
	$(CC) -o client client.c

clean:
	rm -f server_select server_poll client

run-select:
	./server_select 5500

run-poll:
	./server_poll 5500

run-client:
	./client 127.0.0.1 5500

rebuild: clean all

.PHONY: all clean run-select run-poll run-client rebuild