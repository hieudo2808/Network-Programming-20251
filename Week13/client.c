#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./client IPAddress PortNumber\n");
        exit(1);
    }
    
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", server_ip, server_port);
    
    fd_set read_fds;
    int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;
    
    int running = 1;
    
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            perror("select error");
            break;
        }
        
        if (FD_ISSET(sockfd, &read_fds)) {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            
            int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Server disconnected\n");
                } else {
                    perror("recv error");
                }
                running = 0;
                break;
            }
            
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
            
            if (buffer[bytes_received - 1] != '\n') {
                printf("\n");
            }
        }
        
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[BUFFER_SIZE];
            
            if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
                running = 0;
                break;
            }
            
            input[strcspn(input, "\n")] = '\0';
            
            if (strlen(input) == 0) {
                printf("Disconnecting...\n");
                running = 0;
                break;
            }
            
            strcat(input, "\n");
            
            if (send(sockfd, input, strlen(input), 0) < 0) {
                perror("send error");
                running = 0;
                break;
            }
        }
    }
    
    close(sockfd);
    printf("Connection closed\n");
    
    return 0;
}
