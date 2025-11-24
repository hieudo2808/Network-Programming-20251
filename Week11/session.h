#ifndef SESSION_H
#define SESSION_H

#define MAX_USERNAME 50

typedef struct Session {
    char username[MAX_USERNAME];
    char ip_address[50];
    int port;
    int client_fd;
    struct Session* next;
} Session;

// Global session list (protected by semaphore/mutex in server)
extern Session* session_list;

// Add a session
void add_session(const char* username, const char* ip, int port, int client_fd);

// Remove session by username and client_fd
void remove_session(const char* username, int client_fd);

// Check if user is logged in on this client_fd
int is_logged_in(int client_fd);

// Get username for a client_fd
char* get_username_by_fd(int client_fd);

// Get list of logged in users (returns formatted string)
char* get_logged_in_users();

// Free session list
void free_sessions();

#endif
