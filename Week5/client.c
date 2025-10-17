#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <IP> <Port>\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[1024], input[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("Invalid address");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("Failed to connect to server");
        close(sockfd);
        return 1;
    }

    while (1) {
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) break;

        if (send(sockfd, input, strlen(input), 0) < 0) {
            perror("Send failed");
            break;
        }

        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            perror("Receive failed");
            break;
        }
        
        buffer[n] = '\0';
        printf("%s\n", buffer);

        if (strcmp(input, "bye") == 0) break;
    }

    close(sockfd);
    return 0;
}