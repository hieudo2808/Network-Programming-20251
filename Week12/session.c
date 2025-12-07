#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"

Session* session_list = NULL;

void add_session(const char* username, const char* ip, int port, int client_fd) {
    Session* new_session = (Session*)malloc(sizeof(Session));
    strcpy(new_session->username, username);
    strcpy(new_session->ip_address, ip);
    new_session->port = port;
    new_session->client_fd = client_fd;
    new_session->next = session_list;
    session_list = new_session;
}

void remove_session(const char* username, int client_fd) {
    Session* curr = session_list;
    Session* prev = NULL;

    while (curr) {
        if (strcmp(curr->username, username) == 0 && curr->client_fd == client_fd) {
            if (prev) {
                prev->next = curr->next;
            } else {
                session_list = curr->next;
            }
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

int is_logged_in(int client_fd) {
    Session* curr = session_list;
    while (curr) {
        if (curr->client_fd == client_fd) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

char* get_username_by_fd(int client_fd) {
    Session* curr = session_list;
    while (curr) {
        if (curr->client_fd == client_fd) {
            return curr->username;
        }
        curr = curr->next;
    }
    return NULL;
}

char* get_logged_in_users() {
    static char buffer[4096];
    buffer[0] = '\0';
    
    // Use a temporary buffer to track unique usernames
    char unique_users[100][MAX_USERNAME];
    int count = 0;
    
    Session* curr = session_list;
    while (curr) {
        // Check if username already added
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(unique_users[i], curr->username) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            strcpy(unique_users[count], curr->username);
            count++;
            if (strlen(buffer) > 0) {
                strcat(buffer, ", ");
            }
            strcat(buffer, curr->username);
        }
        
        curr = curr->next;
    }
    
    if (strlen(buffer) == 0) {
        strcpy(buffer, "No users logged in");
    }
    
    return buffer;
}

void free_sessions() {
    Session* curr = session_list;
    while (curr) {
        Session* temp = curr;
        curr = curr->next;
        free(temp);
    }
    session_list = NULL;
}
