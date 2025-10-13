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

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    while (1) {
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) break;

        sendto(sockfd, input, strlen(input), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }

    close(sockfd);
    return 0;
}