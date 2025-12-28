#ifndef ACCOUNT_H
#define ACCOUNT_H

#define ACCOUNT_FILE "nguoidung.txt"

typedef struct {
    char username[50];
    char password[50];
    int status;
    int failed_attempts;
} Account;

extern Account accounts[100];
extern int total_acc;

int doc_file_account(void);
void luu_file_account(void);
Account* tim_account(const char* username);
int block_user(const char* username);

#endif