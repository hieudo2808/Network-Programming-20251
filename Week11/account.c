#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "account.h"

Account* load_accounts() {
    FILE* file = fopen(ACCOUNT_FILE, "r");
    if (!file) {
        return NULL;
    }

    Account* head = NULL;
    Account* tail = NULL;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Account* acc = (Account*)malloc(sizeof(Account));
        if (sscanf(line, "%s %s %d", acc->username, acc->password, &acc->status) == 3) {
            acc->failed_attempts = 0;
            acc->next = NULL;

            if (head == NULL) {
                head = acc;
                tail = acc;
            } else {
                tail->next = acc;
                tail = acc;
            }
        } else {
            free(acc);
        }
    }

    fclose(file);
    return head;
}

void save_accounts(Account* head) {
    FILE* file = fopen(ACCOUNT_FILE, "w");
    if (!file) {
        perror("Error saving accounts");
        return;
    }

    Account* curr = head;
    while (curr) {
        fprintf(file, "%s %s %d\n", curr->username, curr->password, curr->status);
        curr = curr->next;
    }

    fclose(file);
}

Account* find_account(Account* head, const char* username) {
    Account* curr = head;
    while (curr) {
        if (strcmp(curr->username, username) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void update_account_status(Account* head, const char* username, int status) {
    Account* acc = find_account(head, username);
    if (acc) {
        acc->status = status;
        save_accounts(head);
    }
}

void reset_failed_attempts(Account* head, const char* username) {
    Account* acc = find_account(head, username);
    if (acc) {
        acc->failed_attempts = 0;
    }
}

int increment_failed_attempts(Account* head, const char* username) {
    Account* acc = find_account(head, username);
    if (acc) {
        acc->failed_attempts++;
        if (acc->failed_attempts >= 3) {
            acc->status = 0; // Lock the account
            save_accounts(head);
            return 1; // Account locked
        }
    }
    return 0; // Not locked yet
}

void free_accounts(Account* head) {
    Account* curr = head;
    while (curr) {
        Account* temp = curr;
        curr = curr->next;
        free(temp);
    }
}
