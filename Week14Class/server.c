#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "account.h"
#include "session.h"
#include "auth_log.h"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

Account* accounts = NULL;

typedef struct {
    int fd;
    char ip[INET_ADDRSTRLEN];
    int port;
    char buf[BUFFER_SIZE];
    int buf_len;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;

int add_client(int fd, const char* ip, int port) {
    if (client_count >= MAX_CLIENTS) return -1;
    clients[client_count].fd = fd;
    strncpy(clients[client_count].ip, ip, INET_ADDRSTRLEN);
    clients[client_count].port = port;
    clients[client_count].buf[0] = '\0';
    clients[client_count].buf_len = 0;
    return client_count++;
}

void remove_client(int idx) {
    clients[idx] = clients[--client_count];
}

void handle_login(int fd, char* username, char* password, const char* ip, int port) {
    if (get_username_by_fd(fd)) {
        send(fd, "Already logged in\n", 18, 0);
        return;
    }
    
    Account* acc = find_account(accounts, username);
    if (!acc) {
        send(fd, "Account does not exist\n", 23, 0);
        log_auth("LOGIN", username, ip, port, "FAIL");
        return;
    }
    if (acc->status == 0) {
        send(fd, "Account is locked\n", 18, 0);
        log_auth("LOGIN", username, ip, port, "FAIL (locked)");
        return;
    }
    if (strcmp(acc->password, password) != 0) {
        send(fd, "Wrong password\n", 15, 0);
        log_auth("LOGIN", username, ip, port, "FAIL");
        if (increment_failed_attempts(accounts, username))
            log_auth("ACCOUNT_LOCKED", username, "", 0, "");
        return;
    }
    
    reset_failed_attempts(accounts, username);
    add_session(username, fd);
    send(fd, "Login successful.\n", 18, 0);
    log_auth("LOGIN", username, ip, port, "SUCCESS");
}

void handle_logout(int fd, const char* ip, int port) {
    char* user = get_username_by_fd(fd);
    if (!user) {
        send(fd, "Not logged in\n", 14, 0);
        return;
    }
    char username[MAX_USERNAME];
    strcpy(username, user);
    remove_session(fd);
    send(fd, "Logout successful.\n", 19, 0);
    log_auth("LOGOUT", username, ip, port, "");
}

void handle_register(int fd, char* username, char* password, const char* ip, int port) {
    int r = register_account(&accounts, username, password);
    if (r == 0) {
        send(fd, "Registration successful.\n", 25, 0);
        log_auth("REGISTER", username, ip, port, "SUCCESS");
    } else if (r == -1)
        send(fd, "Username already exists\n", 24, 0);
    else
        send(fd, "Registration failed\n", 20, 0);
}

void handle_who(int fd) {
    if (!get_username_by_fd(fd)) {
        send(fd, "Not logged in\n", 14, 0);
        return;
    }
    char resp[BUFFER_SIZE];
    int len = sprintf(resp, "LIST %s\n", get_logged_in_users());
    send(fd, resp, len, 0);
}

void handle_help(int fd) {
    const char* h = "Commands: REGISTER/LOGIN <user> <pass>, LOGOUT, WHO, HELP, QUIT\n";
    send(fd, h, strlen(h), 0);
}

int process_cmd(int fd, char* buf, const char* ip, int port) {
    buf[strcspn(buf, "\r\n")] = 0;
    
    char cmd[50], arg1[MAX_USERNAME], arg2[MAX_PASSWORD];
    int n = sscanf(buf, "%s %s %s", cmd, arg1, arg2);
    if (n < 1) return 0;
    
    for (int i = 0; cmd[i]; i++) cmd[i] = toupper(cmd[i]);
    
    if (strcmp(cmd, "LOGIN") == 0 && n == 3) {
        handle_login(fd, arg1, arg2, ip, port);
    } else if (strcmp(cmd, "LOGOUT") == 0) {
        handle_logout(fd, ip, port);
    } else if (strcmp(cmd, "REGISTER") == 0 && n == 3) {
        handle_register(fd, arg1, arg2, ip, port);
    } else if (strcmp(cmd, "WHO") == 0) {
        handle_who(fd);
    } else if (strcmp(cmd, "HELP") == 0) {
        handle_help(fd);
    } else if (strcmp(cmd, "QUIT") == 0) {
        if (get_username_by_fd(fd)) handle_logout(fd, ip, port);
        send(fd, "Goodbye!\n", 9, 0);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./server PortNumber\n");
        exit(1);
    }
    
    int port = atoi(argv[1]);
    accounts = load_accounts();
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);
    
    printf("Server (non-blocking) on port %d\n", port);
    
    fd_set master, readfds;
    FD_ZERO(&master);
    FD_SET(server_fd, &master);
    int maxfd = server_fd;
    
    while (1) {
        readfds = master;
        select(maxfd + 1, &readfds, NULL, NULL, NULL);
        
        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in cli_addr;
            socklen_t len = sizeof(cli_addr);
            int fd = accept(server_fd, (struct sockaddr*)&cli_addr, &len);
            if (fd >= 0) {
                int flags = fcntl(fd, F_GETFL, 0);
                fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
                int p = ntohs(cli_addr.sin_port);
                add_client(fd, ip, p);
                FD_SET(fd, &master);
                if (fd > maxfd) maxfd = fd;
                printf("[+] %s:%d\n", ip, p);
                send(fd, "Type HELP for commands.\n", 24, 0);
            }
        }
        
        for (int i = 0; i < client_count; i++) {
            int fd = clients[i].fd;
            if (!FD_ISSET(fd, &readfds)) continue;
            
            char tmp[BUFFER_SIZE];
            int n = recv(fd, tmp, sizeof(tmp) - 1, 0);
            
            if (n <= 0) {
                printf("[-] %s:%d\n", clients[i].ip, clients[i].port);
                remove_session(fd);
                close(fd);
                FD_CLR(fd, &master);
                remove_client(i--);
                continue;
            }
            
            tmp[n] = '\0';
            strncat(clients[i].buf, tmp, BUFFER_SIZE - clients[i].buf_len - 1);
            clients[i].buf_len += n;
            
            char* nl;
            while ((nl = strchr(clients[i].buf, '\n'))) {
                *nl = '\0';
                printf("[%s:%d] %s\n", clients[i].ip, clients[i].port, clients[i].buf);
                
                if (process_cmd(fd, clients[i].buf, clients[i].ip, clients[i].port)) {
                    close(fd);
                    FD_CLR(fd, &master);
                    remove_client(i--);
                    break;
                }
                
                int len = nl - clients[i].buf + 1;
                memmove(clients[i].buf, nl + 1, clients[i].buf_len - len + 1);
                clients[i].buf_len -= len;
            }
        }
    }
    
    return 0;
}
