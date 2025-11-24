#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int sockfd;
int running = 1;

void* receive_thread(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            printf("Connection closed by server\n");
            running = 0;
            break;
        }
        
        printf("%s", buffer);
        fflush(stdout);
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPAddress PortNumber\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Connect to server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server %s:%d\n", server_ip, server_port);
    
    // Create thread for receiving messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_thread, NULL) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Main loop for sending messages
    char input[BUFFER_SIZE];
    while (running) {
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        // Check if empty input
        if (strlen(input) == 0) {
            continue;
        }
        
        // Send to server
        strcat(input, "\n");
        if (send(sockfd, input, strlen(input), 0) < 0) {
            perror("Send failed");
            break;
        }
        
        // Check if quit command
        char command[50];
        sscanf(input, "%s", command);
        for (int i = 0; command[i]; i++) {
            command[i] = toupper(command[i]);
        }
        if (strcmp(command, "QUIT") == 0) {
            sleep(1); // Give time to receive goodbye message
            break;
        }
    }
    
    running = 0;
    close(sockfd);
    pthread_join(recv_thread, NULL);
    
    printf("Disconnected from server\n");
    
    return 0;
}
