#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "account.h"
#include "session.h"
#include "handler.h"

#define MAX_FDS 100

struct pollfd fds[MAX_FDS];
int nfds = 0;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    
    if (doc_file_account() < 0) return 1;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Loi bind");
        return 1;
    }

    listen(server_sock, 10);

    printf("Server dang chay tai port %d...\n", port);

    fds[0].fd = server_sock;
    fds[0].events = POLLIN;
    nfds = 1;

    while (1) {
        int ret = poll(fds, nfds, -1);

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in cli_addr;
            int len = sizeof(cli_addr);
            int conn = accept(server_sock, (struct sockaddr*)&cli_addr, (socklen_t*)&len);
            
            if (conn >= 0) {
                printf("Ket noi moi: %d\n", conn);
                if (nfds < MAX_FDS) {
                    fds[nfds].fd = conn;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    them_session(conn);
                    send(conn, "Hello. Vui long dang nhap: <user> <pass>\n", 40, 0);
                } else {
                    close(conn);
                }
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buff[1024];
                memset(buff, 0, 1024);
                int n = recv(fds[i].fd, buff, 1024, 0);
                
                if (n <= 0) {
                    printf("Client %d da ngat ket noi.\n", fds[i].fd);
                    close(fds[i].fd);
                    xoa_session(fds[i].fd);
                    
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                } else {
                    process_msg(fds[i].fd, buff);
                }
            }
        }
    }
    
    return 0;
}