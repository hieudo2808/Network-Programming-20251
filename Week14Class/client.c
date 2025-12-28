#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./client IPAddress PortNumber\n");
        exit(EXIT_FAILURE);
    }
    
    int sockfd;
    char input[BUFFER_SIZE];
    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    
    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
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

    while (1) {
        char response[BUFFER_SIZE];
        int bytes_received = recv(sockfd, response, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Receive failed");
            break;
        }
        response[bytes_received] = '\0';
        printf("%s", response);

        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0)
            continue;
        
        strcat(input, "\n");
        if (send(sockfd, input, strlen(input), 0) < 0) {
            perror("Send failed");
            break;
        }
        
        char command[50];
        sscanf(input, "%s", command);
        for (int i = 0; command[i]; i++) 
            command[i] = toupper(command[i]);
        if (strcmp(command, "QUIT") == 0) 
            break;
        
    }
    
    close(sockfd);    
    return 0;
}
