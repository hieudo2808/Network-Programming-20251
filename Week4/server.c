#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "FileHandler/fileprocess.h"
#include "Validation/validation.h"
#include "LinkedList/session.h"
#include "LinkedList/account.h"
#include "ServerHandler/serverHandler.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <Port>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in servAddr, cliAddr;
    char buffer[1024];
    socklen_t len;

    List *accounts = NULL;
    SessionList *sessions = NULL;

    loadData(&accounts);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

    while (1) {
        len = sizeof(cliAddr);
        int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&cliAddr, &len);
        buffer[n] = '\0';

        SessionList *session = findSessionByAddr(&sessions, &cliAddr);
        if (!session) {
            session = createSessionForAddr(&sessions, &cliAddr);
            if (!session) {
                char *errMsg = "Server error\n";
                sendto(sockfd, errMsg, strlen(errMsg), 0, (struct sockaddr*)&cliAddr, len);
                continue;
            }
        }

        char response[1024];
        SessionData *sd = &session->sessionData;

        if (sd->currentUser != NULL) {
            if (strcmp(buffer, "bye") == 0) {
                strcpy(response, "Goodbye");
                sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&cliAddr, len);
                printf("User %s signed out\n", sd->currentUser->username);
                removeSessionByAddr(&sessions, &cliAddr);
                continue;
            } 

            authenticatedUserHandler(sd, buffer, response, accounts);
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&cliAddr, len);
        } 
        else if (strlen(sd->tempUsername) > 0) {
            loginHandler(sd, accounts, buffer, response);
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&cliAddr, len);
        } 
        else {
            strcpy(sd->tempUsername, buffer);
            strcpy(response, "Insert password");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr*)&cliAddr, len);
        }
    }

    close(sockfd);
    saveAllData(accounts);
    freeAllList(accounts);
    freeAllSessions(sessions);
    return 0;
}