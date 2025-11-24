#ifndef ACCOUNT_H
#define ACCOUNT_H

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define ACCOUNT_FILE "account.txt"

typedef struct Account {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int status; // 0: locked, 1: active
    int failed_attempts; // track failed login attempts
    struct Account* next;
} Account;

// Load accounts from file
Account* load_accounts();

// Save accounts to file
void save_accounts(Account* head);

// Find account by username
Account* find_account(Account* head, const char* username);

// Update account status
void update_account_status(Account* head, const char* username, int status);

// Reset failed attempts
void reset_failed_attempts(Account* head, const char* username);

// Increment failed attempts
int increment_failed_attempts(Account* head, const char* username);

// Free account list
void free_accounts(Account* head);

#endif
