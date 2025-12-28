#ifndef SESSION_H
#define SESSION_H

#define MAX_CLIENTS 100

typedef struct {
    int fd;
    char username[50];
    int is_login;
    char tmp_pass[50];
} Session;

extern Session sessions[MAX_CLIENTS];
extern int so_luong_client;

void xoa_session(int fd);
int them_session(int fd);
Session* get_session(int fd);

#endif