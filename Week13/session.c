#include <stdio.h>
#include <string.h>
#include "session.h"

Session sessions[MAX_CLIENTS];
int so_luong_client = 0;

Session* get_session(int fd) {
    for (int i = 0; i < so_luong_client; i++) {
        if (sessions[i].fd == fd) return &sessions[i];
    }
    return NULL;
}

void xoa_session(int fd) {
    int i, j;
    for (i = 0; i < so_luong_client; i++) {
        if (sessions[i].fd == fd) {
            for (j = i; j < so_luong_client - 1; j++) {
                sessions[j] = sessions[j + 1];
            }
            so_luong_client--;
            printf("Da xoa session fd=%d\n", fd);
            return;
        }
    }
}

int them_session(int fd) {
    if (so_luong_client >= MAX_CLIENTS) return -1;
    
    sessions[so_luong_client].fd = fd;
    sessions[so_luong_client].is_login = 0;
    strcpy(sessions[so_luong_client].username, "");
    
    so_luong_client++;
    return 0;
}