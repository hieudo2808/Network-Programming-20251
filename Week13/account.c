#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "account.h"

Account accounts[100];
int total_acc = 0;

int doc_file_account(void) {
    FILE* f = fopen(ACCOUNT_FILE, "r");
    if (f == NULL) {
        printf("Loi mo file account!\n");
        return -1;
    }
    
    total_acc = 0;
    while (!feof(f)) {
        Account acc;
        if (fscanf(f, "%s %s %d", acc.username, acc.password, &acc.status) == 3) {
            acc.failed_attempts = 0;
            accounts[total_acc] = acc;
            total_acc++;
        }
    }
    
    fclose(f);
    printf("Da load xong %d tai khoan.\n", total_acc);
    return 0;
}

void luu_file_account(void) {
    FILE* f = fopen(ACCOUNT_FILE, "w");
    if (f == NULL) return;
    
    for (int i = 0; i < total_acc; i++) {
        fprintf(f, "%s %s %d\n", accounts[i].username, accounts[i].password, accounts[i].status);
    }
    fclose(f);
}

Account* tim_account(const char* username) {
    for (int i = 0; i < total_acc; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return &accounts[i];
        }
    }
    return NULL;
}