#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"

static Session* sessions = NULL;

void add_session(const char* username, int client_fd) {
    Session* s = malloc(sizeof(Session));
    strcpy(s->username, username);
    s->client_fd = client_fd;
    s->next = sessions;
    sessions = s;
}

void remove_session(int client_fd) {
    Session **pp = &sessions, *cur;
    while ((cur = *pp)) {
        if (cur->client_fd == client_fd) {
            *pp = cur->next;
            free(cur);
            return;
        }
        pp = &cur->next;
    }
}

char* get_username_by_fd(int client_fd) {
    for (Session* s = sessions; s; s = s->next)
        if (s->client_fd == client_fd) return s->username;
    return NULL;
}

char* get_logged_in_users() {
    static char buf[2048];
    buf[0] = '\0';
    for (Session* s = sessions; s; s = s->next) {
        if (buf[0]) strcat(buf, ", ");
        strcat(buf, s->username);
    }
    return buf[0] ? buf : "No users logged in";
}
