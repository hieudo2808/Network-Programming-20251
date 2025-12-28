#pragma once

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define ACCOUNT_FILE "account.txt"

typedef struct Account {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int status;
    int failed_attempts;
    struct Account* next;
} Account;

Account* load_accounts();
void save_accounts(Account* head);
Account* find_account(Account* head, const char* username);
void reset_failed_attempts(Account* head, const char* username);
int increment_failed_attempts(Account* head, const char* username);
int register_account(Account** head, const char* username, const char* password);