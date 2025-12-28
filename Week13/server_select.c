#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "account.h"
#include "session.h"
#include "handler.h"

#define BUFF_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    if (doc_file_account() < 0) exit(1);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Loi tao socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Loi bind");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Loi listen");
        exit(1);
    }

    printf("Server Select dang chay o port %d\n", port);

    fd_set read_fds, master_fds;
    int max_fd = server_fd;

    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);

    while (1) {
        read_fds = master_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Loi select");
            break;
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            int len = sizeof(client_addr);
            int new_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&len);

            if (new_fd >= 0) {
                printf("Co ket noi moi: %d\n", new_fd);
                
                FD_SET(new_fd, &master_fds);
                if (new_fd > max_fd) max_fd = new_fd;

                if (them_session(new_fd) < 0) {
                    printf("Server day, tu choi ket noi %d\n", new_fd);
                    close(new_fd);
                    FD_CLR(new_fd, &master_fds);
                } else {
                    send(new_fd, "Hello! Hay dang nhap: <user> <pass>\n", 36, 0);
                }
            }
        }

        for (int i = 0; i < so_luong_client; i++) {
            int fd = sessions[i].fd;
            
            if (FD_ISSET(fd, &read_fds)) {
                char buf[BUFF_SIZE];
                memset(buf, 0, BUFF_SIZE);
                int n = recv(fd, buf, sizeof(buf) - 1, 0);

                if (n <= 0) {
                    printf("Client %d da thoat\n", fd);
                    close(fd);
                    FD_CLR(fd, &master_fds);
                    
                    xoa_session(fd);
                    i--;
                } else {
                    process_msg(fd, buf);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}