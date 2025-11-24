#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LOGIN 1
#define TEXT 2
#define SERVER_PORT 8080

typedef struct {
    int type;
    char data[1024];
} Message;

int main() {
    int sockfd;
    struct sockaddr_in serv;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket create failed");
        return 1;
    }

    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT);
    serv.sin_addr.s_addr = inet_addr("192.168.1.114");

    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("Connect to server failed");
        return 1;
    }

    printf("Connected to server.\n");

    Message msg;

    msg.type = LOGIN;
    printf("Enter login name: ");
    fgets(msg.data, sizeof(msg.data), stdin);
    msg.data[strcspn(msg.data, "\n")] = 0;

    send(sockfd, &msg, sizeof(msg), 0);

    int recv_len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (recv_len <= 0) {
        close(sockfd);
        return 0;
    }

    buffer[recv_len] = '\0';
    if (strcmp(buffer, "Logged in") != 0) {
        printf("Login failed: %s\n", buffer);
        close(sockfd);
        return 0;
    }

    printf("Logged in successfully\n");

    while (1) {
        msg.type = TEXT;
        fgets(msg.data, sizeof(msg.data), stdin);
        msg.data[strcspn(msg.data, "\n")] = 0;

        send(sockfd, &msg, sizeof(msg), 0);

        recv_len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (recv_len <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        buffer[recv_len] = '\0';
        printf("Server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
