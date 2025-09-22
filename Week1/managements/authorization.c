#include "../management_handler.h"

void printAdminMenu() {
    printf("ADMIN MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. View user accounts list\n");
    printf("2. Delete user account\n");
    printf("3. Reset user password\n");
}

void viewLoginHistory() {
    if (currentUser == NULL) {
        printf("Please login first\n");
        return;
    }

    loadHistory(currentUser->username);
}
void authorization(List **list) {
    if (currentUser == NULL) {
        printf("Please login first\n");
        return;
    }

    if (strcmp(currentUser->role, "admin") != 0) {
        printf("Access denied\n");
        return;
    }

    printAdminMenu();
    int adminChoice;
    scanf("%d", &adminChoice);
    wipeBufferOut();

    if (adminChoice == 1)
        printList(*list);
    else if (adminChoice == 2) {
        char username[50];
        printf("Please enter username: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;
        if (strchr(username, '\n') == NULL) wipeBufferOut();

        if (!validateUsername(username)) {
            printf("Username is invalid\n");
            return;
        }

        List *userNode = checkExist(list, username);
        if (userNode == NULL) {
            printf("Cannot find account\n");
            return;
        }

        if (strcmp(username, currentUser->username) == 0) {
            printf("Failed: Cant delete your own account\n");
            return;
        }

        deleteNode(list, username);
        saveAllData(*list);
        printf("Account deleted successfully\n");
    } else if (adminChoice == 3) {
        char username[50];
        printf("Please enter username to reset password: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;
        if (strchr(username, '\n') == NULL) wipeBufferOut();

        if (!validateUsername(username)) {
            printf("Username is invalid\n");
            return;
        }

        List *userNode = checkExist(list, username);
        if (userNode == NULL) {
            printf("Account not exist\n");
            return;
        }

        char newPassword[50];
        printf("Please enter new password: ");
        fgets(newPassword, sizeof(newPassword), stdin);
        newPassword[strcspn(newPassword, "\n")] = 0;
        if (strchr(newPassword, '\n') == NULL) wipeBufferOut();

        if (!validatePassword(newPassword)) {
            printf(
                "Password must be at least 8 characters long and contain both "
                "letters and numbers\n");
            return;
        }

        strcpy(userNode->userData.password, newPassword);
        saveAllData(*list);
        printf("Password reset successfully\n");
    } else {
        printf("Invalid choice\n");
    }
}
