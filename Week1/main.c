#include <stdio.h>

#include "fileprocess.h"
#include "linklist.h"
#include "management_handler.h"

void printMenu() {
    printf("USER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Register\n");
    printf("2. Sign in \n");
    printf("3. Change password\n");
    printf("4. Update account info\n");
    printf("5. Reset password\n");
    printf("6. View login history\n");
    printf("7. Authorization\n");
    printf("8. Sign out\n");
    printf("Your choice (1-8, other to quit): ");
}

int main() {
    List *userData = NULL;
    loadData(&userData);

    while (1) {
        printMenu();

        int choice;
        scanf("%d", &choice);
        wipeBufferOut();

        if (choice < 1 || choice > 8) break;

        switch (choice) {
            case 1:
                newUserRegister(&userData);
                break;

            case 2:
                login(&userData);
                break;

            case 3:
                changePassword(&userData);
                break;

            case 4:
                updateAccountInfo(&userData);
                break;

            case 5:
                resetPassword(&userData);
                break;

            case 6:
                viewLoginHistory();
                break;

            case 7:
                authorization(&userData);
                break;

            case 8:
                signOut();
                break;
        }
    }

    freeAllList(userData);
    return 0;
}