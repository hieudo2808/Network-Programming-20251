#pragma once

#include <arpa/inet.h>

#include "account.h"

typedef struct {
    char clientIP[INET_ADDRSTRLEN];
    int clientPort;
    Data *currentUser;
    char tempUsername[50];
    int loginFailedCount;
} SessionData;

typedef struct SessionList {
    SessionData sessionData;
    struct SessionList *next;
} SessionList;

SessionList *findSessionByAddr(SessionList **list, struct sockaddr_in *addr);
SessionList *createSessionForAddr(SessionList **list, struct sockaddr_in *addr);
void removeSessionByAddr(SessionList **list, struct sockaddr_in *addr);
void freeAllSessions(SessionList *list);