#include "session.h"
#include <stdlib.h>
#include <string.h>

SessionList* findSessionByAddr(SessionList **list, struct sockaddr_in *addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr->sin_port);
    
    SessionList *current = *list;
    while (current != NULL) {
        if (strcmp(current->sessionData.clientIP, ip) == 0 && 
            current->sessionData.clientPort == port) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

SessionList* createSessionForAddr(SessionList **list, struct sockaddr_in *addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr->sin_port);
    
    SessionList *newSession = (SessionList*)malloc(sizeof(SessionList));
    if (!newSession) return NULL;
    
    strcpy(newSession->sessionData.clientIP, ip);
    newSession->sessionData.clientPort = port;
    newSession->sessionData.currentUser = NULL;
    newSession->sessionData.tempUsername[0] = '\0';
    newSession->sessionData.failedAttempts = 0;
    newSession->next = *list;
    *list = newSession;
    
    return newSession;
}

void removeSessionByAddr(SessionList **list, struct sockaddr_in *addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr->sin_port);
    
    SessionList *current = *list;
    SessionList *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->sessionData.clientIP, ip) == 0 && 
            current->sessionData.clientPort == port) {
            if (prev == NULL) {
                *list = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void freeAllSessions(SessionList *list) {
    SessionList *current = list;
    while (current != NULL) {
        SessionList *temp = current;
        current = current->next;
        free(temp);
    }
}