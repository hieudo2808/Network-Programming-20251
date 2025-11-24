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

void handle_sigchld(int sig) {
    // Clean up zombie processes
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void send_response(int client_fd, const char* response) {
    send(client_fd, response, strlen(response), 0);
}

void handle_login(int client_fd, char* username, char* password, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    // Check if already logged in on this connection
    if (is_logged_in(client_fd)) {
        sprintf(response, "ERR Already logged in\n");
        send_response(client_fd, response);
        return;
    }
    
    // Find account
    Account* acc = find_account(accounts, username);
    
    if (!acc) {
        sprintf(response, "ERR Account does not exist\n");
        send_response(client_fd, response);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account does not exist)");
        return;
    }
    
    if (acc->status == 0) {
        sprintf(response, "ERR Account is locked\n");
        send_response(client_fd, response);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account locked)");
        return;
    }
    
    if (strcmp(acc->password, password) != 0) {
        sprintf(response, "ERR Wrong password\n");
        send_response(client_fd, response);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (wrong password)");
        
        // Increment failed attempts
        if (increment_failed_attempts(accounts, username)) {
            log_auth("ACCOUNT_LOCKED", username, "", 0, "");
        }
        return;
    }
    
    // Successful login
    reset_failed_attempts(accounts, username);
    add_session(username, client_ip, client_port, client_fd);
    sprintf(response, "OK Login successful. Welcome %s!\n", username);
    send_response(client_fd, response);
    log_auth("LOGIN", username, client_ip, client_port, "SUCCESS");
}

void handle_logout(int client_fd, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    if (!is_logged_in(client_fd)) {
        sprintf(response, "ERR Not logged in\n");
        send_response(client_fd, response);
        return;
    }
    
    char* username = get_username_by_fd(client_fd);
    if (username) {
        char user_copy[MAX_USERNAME];
        strcpy(user_copy, username);
        remove_session(username, client_fd);
        sprintf(response, "OK Logout successful. Goodbye!\n");
        send_response(client_fd, response);
        log_auth("LOGOUT", user_copy, client_ip, client_port, "");
    }
}

void handle_who(int client_fd) {
    char response[BUFFER_SIZE];
    
    if (!is_logged_in(client_fd)) {
        sprintf(response, "ERR Not logged in\n");
        send_response(client_fd, response);
        return;
    }
    
    char* users = get_logged_in_users();
    sprintf(response, "LIST %s\n", users);
    send_response(client_fd, response);
}

void handle_help(int client_fd) {
    char response[BUFFER_SIZE];
    sprintf(response, 
        "OK Available commands:\n"
        "  LOGIN <username> <password> - Login to your account\n"
        "  LOGOUT - Logout from current session\n"
        "  WHO - List all logged in users\n"
        "  HELP - Show this help message\n"
        "  QUIT - Close connection\n");
    send_response(client_fd, response);
}

void handle_client(int client_fd, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(client_addr.sin_port);
    
    printf("Client connected from %s:%d\n", client_ip, client_port);
    
    // Send welcome message
    send_response(client_fd, "OK Welcome! Type HELP for available commands.\n");
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        // Remove newline
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        printf("Received from %s:%d: %s\n", client_ip, client_port, buffer);
        
        // Parse command
        char command[50], arg1[MAX_USERNAME], arg2[MAX_PASSWORD];
        int num_args = sscanf(buffer, "%s %s %s", command, arg1, arg2);
        
        if (num_args < 1) {
            continue;
        }
        
        // Convert command to uppercase for comparison
        for (int i = 0; command[i]; i++) {
            command[i] = toupper(command[i]);
        }
        
        if (strcmp(command, "LOGIN") == 0) {
            if (num_args == 3) {
                handle_login(client_fd, arg1, arg2, client_ip, client_port);
            } else {
                send_response(client_fd, "ERR Usage: LOGIN <username> <password>\n");
            }
        } else if (strcmp(command, "LOGOUT") == 0) {
            handle_logout(client_fd, client_ip, client_port);
        } else if (strcmp(command, "WHO") == 0) {
            handle_who(client_fd);
        } else if (strcmp(command, "HELP") == 0) {
            handle_help(client_fd);
        } else if (strcmp(command, "QUIT") == 0) {
            if (is_logged_in(client_fd)) {
                handle_logout(client_fd, client_ip, client_port);
            }
            send_response(client_fd, "OK Goodbye!\n");
            break;
        } else {
            send_response(client_fd, "ERR Unknown command. Type HELP for available commands.\n");
        }
    }
    
    // Clean up session if still logged in
    if (is_logged_in(client_fd)) {
        char* username = get_username_by_fd(client_fd);
        if (username) {
            remove_session(username, client_fd);
            log_auth("LOGOUT", username, client_ip, client_port, "");
        }
    }
    
    close(client_fd);
    printf("Client disconnected from %s:%d\n", client_ip, client_port);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
    
    // Load accounts
    accounts = load_accounts();
    if (!accounts) {
        fprintf(stderr, "Warning: Could not load accounts from file\n");
    }
    
    // Setup signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", port);
    
    // Accept connections
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Fork to handle client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_fd);
            continue;
        }
        
        if (pid == 0) {
            // Child process
            close(server_fd);
            handle_client(client_fd, client_addr);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(client_fd);
        }
    }
    
    // Clean up
    close(server_fd);
    free_accounts(accounts);
    free_sessions();
    
    return 0;
}
