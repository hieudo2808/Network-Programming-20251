#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include "FileHandler/fileprocess.h"
#include "Validation/validation.h"
#include "LinkedList/session.h"
#include "LinkedList/account.h"
#include "ServerHandler/serverHandler.h"

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    if (argc != 2) {
        printf("Usage: ./server <Port>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int sockfd, connfd;
    struct sockaddr_in servAddr, cliAddr;
    char buffer[1024];
    socklen_t len;

    List *accounts = NULL;
    SessionList *sessions = NULL;

    loadData(&accounts);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error when creating socket");
        return 1;
    }

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("Error when binding socket");
        return 1;
    }

    printf("Server running on port %d...\n", port);
    if (listen(sockfd, 5) != 0)
    {
        perror("LISTEN failed\n");
        return 1;
    }

    while (1) {
        len = sizeof(cliAddr);
        connfd = accept(sockfd, (struct sockaddr*)&cliAddr, &len);
        if (connfd == -1)
        {
            perror("Failed to establish connection");
            continue;
        }

        printf("Client connected from %s:%d\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));

        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
            continue;
        }
        
        if (pid == 0) {
            close(sockfd);

            SessionList *session = findSessionByAddr(&sessions, &cliAddr);
            if (!session) {
                session = createSessionForAddr(&sessions, &cliAddr);
                if (!session) {
                    char *errMsg = "Server error";
                    send(connfd, errMsg, strlen(errMsg), 0);
                    close(connfd);
                    exit(1);
                }
            }

            while (1) {
                int n = recv(connfd, buffer, sizeof(buffer) - 1, 0);
                if (n < 0) {
                    perror("Failed to receive data");
                    break;
                }
                buffer[n] = '\0';

                char response[1024];
                SessionData *sd = &session->sessionData;

                if (sd->currentUser != NULL) {
                    if (strcmp(buffer, "bye") == 0) {
                        snprintf(response, sizeof(response), "Goodbye %s", sd->currentUser->username);
                        send(connfd, response, strlen(response), 0);
                        printf("User %s signed out\n", sd->currentUser->username);
                        removeSessionByAddr(&sessions, &cliAddr);
                        break;
                    } 

                    authenticatedUserHandler(sd, buffer, response, accounts);
                    send(connfd, response, strlen(response), 0);
                } 
                else if (strlen(sd->tempUsername) > 0) {
                    loginHandler(sd, accounts, buffer, response);
                    send(connfd, response, strlen(response), 0);
                } 
                else {
                    strcpy(sd->tempUsername, buffer);
                    strcpy(response, "Insert password");
                    send(connfd, response, strlen(response), 0);
                }
            }
            
            close(connfd);
            printf("Client disconnected\n");
            exit(0);
        }
        
        close(connfd);
    }

    close(sockfd);
    saveAllData(accounts);
    freeAllList(accounts);
    freeAllSessions(sessions);
    return 0;
}