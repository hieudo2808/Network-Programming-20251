#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "account.h"
#include "session.h"
#include "auth_log.h"

#define BUFFER_SIZE 1024

Account* accounts = NULL;

void sigchld_handler(int signo) {
    pid_t pid;
    pid = waitpid(-1, NULL, WNOHANG);
    printf("Child %d terminated\n", pid);
}

void handle_login(int clientfd, char* username, char* password, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    if (is_logged_in(clientfd)) {
        sprintf(response, "Already logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    Account* acc = find_account(accounts, username);
    
    if (!acc) {
        sprintf(response, "Account does not exist\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account does not exist)");
        return;
    }
    
    if (acc->status == 0) {
        sprintf(response, "Account is locked\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account locked)");
        return;
    }
    
    if (strcmp(acc->password, password) != 0) {
        sprintf(response, "Wrong password\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (wrong password)");
        
        if (increment_failed_attempts(accounts, username)) {
            log_auth("ACCOUNT_LOCKED", username, "", 0, "");
        }
        return;
    }
    
    reset_failed_attempts(accounts, username);
    add_session(username, client_ip, client_port, clientfd);
    strcpy(response, "Login successful.\n");
    send(clientfd, response, strlen(response), 0);
    log_auth("LOGIN", username, client_ip, client_port, "SUCCESS");
}

void handle_logout(int clientfd, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    if (!is_logged_in(clientfd)) {
        sprintf(response, "Not logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    char* username = get_username_by_fd(clientfd);
    if (username) {
        char user_copy[MAX_USERNAME];
        strcpy(user_copy, username);
        remove_session(username, clientfd);
        strcpy(response, "Logout successful.\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("LOGOUT", user_copy, client_ip, client_port, "");
    }
}

void handle_register(int clientfd, char* username, char* password, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    int result = register_account(&accounts, username, password);
    
    if (result == 0) {
        strcpy(response, "Registration successful.\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("REGISTER", username, client_ip, client_port, "SUCCESS");
    } else if (result == -1) {
        strcpy(response, "Username already exists\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (username exists)");
    } else if (result == -2) {
        strcpy(response, "Invalid username\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (invalid username)");
    } else if (result == -3) {
        strcpy(response, "Invalid password\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (invalid password)");
    } else {
        strcpy(response, "Registration failed\n");
        send(clientfd, response, strlen(response), 0);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (system error)");
    }
}

void handle_who(int clientfd) {
    char response[BUFFER_SIZE];
    
    if (!is_logged_in(clientfd)) {
        sprintf(response, "Not logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    char* users = get_logged_in_users();
    sprintf(response, "LIST %s\n", users);
    send(clientfd, response, strlen(response), 0);
}

void handle_help(int clientfd) {
    char response[BUFFER_SIZE];
    sprintf(response, 
        "Available commands:\n"
        "  REGISTER <username> <password> - Register a new account\n"
        "  LOGIN <username> <password> - Login to your account\n"
        "  LOGOUT - Logout from current session\n"
        "  WHO - List all logged in users\n"
        "  HELP - Show this help message\n"
        "  QUIT - Close connection\n");
    send(clientfd, response, strlen(response), 0);
}

void handle_client(int clientfd, struct sockaddr_in clientAddr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    
    inet_ntop(AF_INET, &clientAddr.sin_addr, client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(clientAddr.sin_port);
    
    printf("Client connected from %s:%d\n", client_ip, client_port);
    send(clientfd, "Type HELP for available commands.\n", strlen("Type HELP for available commands.\n"), 0);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(clientfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        printf("Received from %s:%d: %s\n", client_ip, client_port, buffer);
        
        char command[50], arg1[MAX_USERNAME], arg2[MAX_PASSWORD];
        int num_args = sscanf(buffer, "%s %s %s", command, arg1, arg2);
        
        if (num_args < 1) {
            continue;
        }
        
        for (int i = 0; command[i]; i++) {
            command[i] = toupper(command[i]);
        }
        
        if (strcmp(command, "REGISTER") == 0) {
            if (num_args == 3) {
                handle_register(clientfd, arg1, arg2, client_ip, client_port);
            } else {
                send(clientfd, "Usage: REGISTER <username> <password>\n", strlen("Usage: REGISTER <username> <password>\n"), 0);
            }
        } else if (strcmp(command, "LOGIN") == 0) {
            if (num_args == 3) {
                handle_login(clientfd, arg1, arg2, client_ip, client_port);
            } else {
                send(clientfd, "Usage: LOGIN <username> <password>\n", strlen("Usage: LOGIN <username> <password>\n"), 0);
            }
        } else if (strcmp(command, "LOGOUT") == 0) {
            handle_logout(clientfd, client_ip, client_port);
        } else if (strcmp(command, "WHO") == 0) {
            handle_who(clientfd);
        } else if (strcmp(command, "HELP") == 0) {
            handle_help(clientfd);
        } else if (strcmp(command, "QUIT") == 0) {
            if (is_logged_in(clientfd)) {
                handle_logout(clientfd, client_ip, client_port);
            }
            send(clientfd, "Goodbye!\n", strlen("Goodbye!\n"), 0);
            break;
        }
    }
    
    if (is_logged_in(clientfd)) {
        char* username = get_username_by_fd(clientfd);
        if (username) {
            remove_session(username, clientfd);
            log_auth("LOGOUT", username, client_ip, client_port, "");
        }
    }
    
    close(clientfd);
    printf("Client disconnected from %s:%d\n", client_ip, client_port);
}

int main(int argc, char* argv[]) {
    signal(SIGCHLD, sigchld_handler);

    if (argc != 2) {
        fprintf(stderr, "Usage: ./server PortNumber\n");
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
    
    accounts = load_accounts();
    if (!accounts) {
        fprintf(stderr, "Could not load accounts from file\n");
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", port);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientfd < 0) {
            perror("Accept failed");
            continue;
        }
        
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(clientfd);
            continue;
        }
        
        if (pid == 0) {
            close(sockfd);
            handle_client(clientfd, clientAddr);
            exit(EXIT_SUCCESS);
        } else {
            close(clientfd);
        }
    }
    
    close(sockfd);
    free_accounts(accounts);
    free_sessions();
    
    return 0;
}
