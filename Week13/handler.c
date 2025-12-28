#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>

#include "handler.h"
#include "account.h"
#include "session.h"

#define BUFF_SIZE 1024

int kiem_tra_chuoi(const char* s) {
    for(int i=0; i<strlen(s); i++) {
        if (!isalnum(s[i])) return 0;
    }
    return 1;
}

void tach_mk(const char* src, char* chu, char* so) {
    int c=0, s=0;
    for(int i=0; i<strlen(src); i++) {
        if (isalpha(src[i])) chu[c++] = src[i];
        else if (isdigit(src[i])) so[s++] = src[i];
    }
    chu[c] = '\0';
    so[s] = '\0';
}

void xu_ly_login(int client_fd, Session* sess, char* msg) {
    char user[50], pass[50];
    char resp[BUFF_SIZE];
    
    int n = sscanf(msg, "%s %s", user, pass);
    if (n != 2) {
        strcpy(resp, "Error\n");
        send(client_fd, resp, strlen(resp), 0);
        return;
    }

    Account* acc = tim_account(user);
    if (acc == NULL) {
        sprintf(resp, "Account not ready\n");
        send(client_fd, resp, strlen(resp), 0);
        return;
    }

    if (acc->status == 0) {
        send(client_fd, "Account is blocked.\n", 20, 0);
        return;
    }

    if (strcmp(acc->password, pass) == 0) {
        sess->is_login = 1;
        strcpy(sess->username, user);
        acc->failed_attempts = 0;
        
        sprintf(resp, "OK\n");
        send(client_fd, resp, strlen(resp), 0);
    } else {
        acc->failed_attempts++;
        if (acc->failed_attempts >= 3) {
            acc->status = 0;
            luu_file_account();
            send(client_fd, "Account is blocked\n", 20, 0);
        } else {
            send(client_fd, "Not OK\n", 7, 0);
        }
    }
}

void doi_mat_khau(int fd, Session* sess, char* new_pass) {
    if (kiem_tra_chuoi(new_pass) == 0) {
        send(fd, "Mat khau chi duoc chua chu va so\n", 35, 0);
        return;
    }
    
    Account* acc = tim_account(sess->username);
    if (acc) {
        strcpy(acc->password, new_pass);
        luu_file_account();
        
        char phan_chu[50], phan_so[50];
        tach_mk(new_pass, phan_chu, phan_so);
        
        char msg_cho_minh[BUFF_SIZE];
        char msg_cho_nguoi_khac[BUFF_SIZE];

        sprintf(msg_cho_minh, "%s\n%s\n", phan_chu, phan_so);
        sprintf(msg_cho_nguoi_khac, "\nNew pass: %s\n%s\n", phan_chu, phan_so);

        for (int i = 0; i < so_luong_client; i++) {
            if (sessions[i].is_login && strcmp(sessions[i].username, sess->username) == 0) {
                
                if (sessions[i].fd == fd) {
                    send(fd, msg_cho_minh, strlen(msg_cho_minh), 0);
                } else {
                    send(sessions[i].fd, msg_cho_nguoi_khac, strlen(msg_cho_nguoi_khac), 0);
                }
            }
        }
    }
}

void process_msg(int fd, char* buffer) {
    buffer[strcspn(buffer, "\r\n")] = 0;
    
    Session* sess = get_session(fd);
    if (!sess) return;

    printf("Client %d gui: %s\n", fd, buffer);

    if (sess->is_login == 0) {
        xu_ly_login(fd, sess, buffer);
    } else {
        if (strcmp(buffer, "bye") == 0 || strcmp(buffer, "exit") == 0) {
            send(fd, "Goodbye\n", 8, 0);
            sess->is_login = 0;
            strcpy(sess->username, "");
        } else {
            doi_mat_khau(fd, sess, buffer);
        }
    }
}