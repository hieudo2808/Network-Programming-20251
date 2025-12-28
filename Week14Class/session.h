#ifndef SESSION_H
#define SESSION_H

#include "account.h"

typedef struct Session {
    char username[MAX_USERNAME];
    int client_fd;
    struct Session* next;
} Session;

void add_session(const char* username, int client_fd);
void remove_session(int client_fd);
char* get_username_by_fd(int client_fd);
char* get_logged_in_users();

#endif
