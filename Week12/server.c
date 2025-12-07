#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "account.h"
#include "session.h"
#include "auth_log.h"

#define BUFFER_SIZE 1024

Account* accounts = NULL;
pthread_mutex_t accounts_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sessions_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int clientfd;
    struct sockaddr_in clientAddr;
} client_info_t;

void handle_login(int clientfd, char* username, char* password, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    if (is_logged_in(clientfd)) {
        sprintf(response, "Already logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    pthread_mutex_lock(&accounts_mutex);
    Account* acc = find_account(accounts, username);
    
    if (!acc) {
        pthread_mutex_unlock(&accounts_mutex);
        sprintf(response, "Account does not exist\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account does not exist)");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    
    if (acc->status == 0) {
        pthread_mutex_unlock(&accounts_mutex);
        sprintf(response, "Account is locked\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (account locked)");
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    
    if (strcmp(acc->password, password) != 0) {
        sprintf(response, "Wrong password\n");
        send(clientfd, response, strlen(response), 0);
        
        pthread_mutex_lock(&log_mutex);
        log_auth("LOGIN", username, client_ip, client_port, "FAIL (wrong password)");
        pthread_mutex_unlock(&log_mutex);
        
        if (increment_failed_attempts(accounts, username)) {
            pthread_mutex_lock(&log_mutex);
            log_auth("ACCOUNT_LOCKED", username, "", 0, "");
            pthread_mutex_unlock(&log_mutex);
        }
        pthread_mutex_unlock(&accounts_mutex);
        return;
    }
    
    reset_failed_attempts(accounts, username);
    pthread_mutex_unlock(&accounts_mutex);
    
    pthread_mutex_lock(&sessions_mutex);
    add_session(username, client_ip, client_port, clientfd);
    pthread_mutex_unlock(&sessions_mutex);
    
    strcpy(response, "Login successful.\n");
    send(clientfd, response, strlen(response), 0);
    
    pthread_mutex_lock(&log_mutex);
    log_auth("LOGIN", username, client_ip, client_port, "SUCCESS");
    pthread_mutex_unlock(&log_mutex);
}

void handle_logout(int clientfd, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    pthread_mutex_lock(&sessions_mutex);
    if (!is_logged_in(clientfd)) {
        pthread_mutex_unlock(&sessions_mutex);
        sprintf(response, "Not logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    char* username = get_username_by_fd(clientfd);
    if (username) {
        char user_copy[MAX_USERNAME];
        strcpy(user_copy, username);
        remove_session(username, clientfd);
        pthread_mutex_unlock(&sessions_mutex);
        
        strcpy(response, "Logout successful.\n");
        send(clientfd, response, strlen(response), 0);
        
        pthread_mutex_lock(&log_mutex);
        log_auth("LOGOUT", user_copy, client_ip, client_port, "");
        pthread_mutex_unlock(&log_mutex);
    } else {
        pthread_mutex_unlock(&sessions_mutex);
    }
}

void handle_register(int clientfd, char* username, char* password, const char* client_ip, int client_port) {
    char response[BUFFER_SIZE];
    
    pthread_mutex_lock(&accounts_mutex);
    int result = register_account(&accounts, username, password);
    pthread_mutex_unlock(&accounts_mutex);
    
    if (result == 0) {
        strcpy(response, "Registration successful.\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("REGISTER", username, client_ip, client_port, "SUCCESS");
        pthread_mutex_unlock(&log_mutex);
    } else if (result == -1) {
        strcpy(response, "Username already exists\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (username exists)");
        pthread_mutex_unlock(&log_mutex);
    } else if (result == -2) {
        strcpy(response, "Invalid username\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (invalid username)");
        pthread_mutex_unlock(&log_mutex);
    } else if (result == -3) {
        strcpy(response, "Invalid password\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (invalid password)");
        pthread_mutex_unlock(&log_mutex);
    } else {
        strcpy(response, "Registration failed\n");
        send(clientfd, response, strlen(response), 0);
        pthread_mutex_lock(&log_mutex);
        log_auth("REGISTER", username, client_ip, client_port, "FAIL (system error)");
        pthread_mutex_unlock(&log_mutex);
    }
}

void handle_who(int clientfd) {
    char response[BUFFER_SIZE];
    
    pthread_mutex_lock(&sessions_mutex);
    if (!is_logged_in(clientfd)) {
        pthread_mutex_unlock(&sessions_mutex);
        sprintf(response, "Not logged in\n");
        send(clientfd, response, strlen(response), 0);
        return;
    }
    
    char* users = get_logged_in_users();
    sprintf(response, "LIST %s\n", users);
    pthread_mutex_unlock(&sessions_mutex);
    
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

void* handle_client(void* arg) {
    client_info_t* client_info = (client_info_t*)arg;
    int clientfd = client_info->clientfd;
    struct sockaddr_in clientAddr = client_info->clientAddr;
    free(client_info);
    
    pthread_detach(pthread_self());
    
    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    
    inet_ntop(AF_INET, &clientAddr.sin_addr, client_ip, INET_ADDRSTRLEN);
    client_port = ntohs(clientAddr.sin_port);
    
    printf("Client connected from %s:%d [Thread %lu]\n", client_ip, client_port, pthread_self());
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
            pthread_mutex_lock(&sessions_mutex);
            if (is_logged_in(clientfd)) {
                pthread_mutex_unlock(&sessions_mutex);
                handle_logout(clientfd, client_ip, client_port);
            } else {
                pthread_mutex_unlock(&sessions_mutex);
            }
            send(clientfd, "Goodbye!\n", strlen("Goodbye!\n"), 0);
            break;
        }
    }
    
    pthread_mutex_lock(&sessions_mutex);
    if (is_logged_in(clientfd)) {
        char* username = get_username_by_fd(clientfd);
        if (username) {
            char user_copy[MAX_USERNAME];
            strcpy(user_copy, username);
            remove_session(username, clientfd);
            pthread_mutex_unlock(&sessions_mutex);
            
            pthread_mutex_lock(&log_mutex);
            log_auth("LOGOUT", user_copy, client_ip, client_port, "");
            pthread_mutex_unlock(&log_mutex);
        } else {
            pthread_mutex_unlock(&sessions_mutex);
        }
    } else {
        pthread_mutex_unlock(&sessions_mutex);
    }
    
    close(clientfd);
    printf("Client disconnected from %s:%d [Thread %lu]\n", client_ip, client_port, pthread_self());
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./server PortNumber\n");
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
    
    pthread_mutex_lock(&accounts_mutex);
    accounts = load_accounts();
    pthread_mutex_unlock(&accounts_mutex);
    
    if (!accounts) {
        fprintf(stderr, "Could not load accounts from file\n");
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
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
        
        // Create thread to handle client
        pthread_t thread_id;
        client_info_t* client_info = (client_info_t*)malloc(sizeof(client_info_t));
        client_info->clientfd = clientfd;
        client_info->clientAddr = clientAddr;
        
        if (pthread_create(&thread_id, NULL, handle_client, (void*)client_info) != 0) {
            perror("Thread creation failed");
            close(clientfd);
            free(client_info);
            continue;
        }
    }
    
    close(sockfd);
    
    pthread_mutex_lock(&accounts_mutex);
    free_accounts(accounts);
    pthread_mutex_unlock(&accounts_mutex);
    
    pthread_mutex_lock(&sessions_mutex);
    free_sessions();
    pthread_mutex_unlock(&sessions_mutex);
    
    pthread_mutex_destroy(&accounts_mutex);
    pthread_mutex_destroy(&sessions_mutex);
    pthread_mutex_destroy(&log_mutex);
    
    return 0;
}
