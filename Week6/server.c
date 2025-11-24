#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 2048
#define LOGIN 1
#define TEXT 2
#define SERVER_PORT 8080

typedef struct {
    int type;
    char data[1024];
} Message;

void write_log(const char *username, const char *client, const char *msg) {
    char filename[128];
    sprintf(filename, "%s.txt", username);

    FILE *file = fopen(filename, "a");
    if (!file) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(file, "[%02d-%02d-%04d %02d:%02d:%02d] [%s] %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        client, msg
    );

    fclose(file);
}

int main() {
    int sockfd, clientfd;
    struct sockaddr_in serv, cli;
    socklen_t cli_len = sizeof(cli);
    char logged_user[1000] = "";
    char client_info[25] = "";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Create socket failed");
        return 1;
    }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    listen(sockfd, 5);

    while (1) {
        clientfd = accept(sockfd, (struct sockaddr *)&cli, &cli_len);
        if (clientfd < 0) {
            perror("Accept failed");
            continue;
        }

        sprintf(client_info, "%s:%d", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
        printf("Client connected: %s\n", client_info);

        while (1) {
            Message msg;
            int n = recv(clientfd, &msg, sizeof(msg), 0);
            if (n <= 0) break;

            msg.data[sizeof(msg.data) - 1] = '\0';

            if (msg.type == LOGIN) {
                strcpy(logged_user, msg.data);
                send(clientfd, "Logged in", 9, 0);
                printf("[%s] Logged in as %s\n", client_info, logged_user);
            } else if (msg.type == TEXT) {
                if (strlen(logged_user) == 0)
                    send(clientfd, "ERROR: Not logged in", 20, 0);
                else {
                    write_log(logged_user, client_info, msg.data);
                    printf("[%s:%s] %s\n", logged_user, client_info, msg.data);
                }
            }
        }

        close(clientfd);
        printf("Client %s disconnected.\n", client_info);
    }

    close(sockfd);
    return 0;
}
