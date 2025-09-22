#include "../management_handler.h"

void wipeBufferOut() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void newUserRegister(List **list) {
    Data newUser;

    int validUsername, validPassword, validEmail, validPhone;

    do {
        printf("Please enter username: ");
        fgets(newUser.username, sizeof(newUser.username), stdin);
        if (strchr(newUser.username, '\n') == NULL) wipeBufferOut();
        newUser.username[strcspn(newUser.username, "\n")] = '\0';

        validUsername = validateUsername(newUser.username);
        if (!validUsername) {
            printf("Username is invalid\n");
            continue;
        }

        if (checkExist(list, newUser.username) != NULL) {
            printf("Username already exists\n");
            validUsername = 0;
        }
    } while (!validUsername);

    do {
        printf("Please enter password: ");
        fgets(newUser.password, sizeof(newUser.password), stdin);
        if (strchr(newUser.password, '\n') == NULL) wipeBufferOut();
        newUser.password[strcspn(newUser.password, "\n")] = '\0';

        validPassword = validatePassword(newUser.password);
        if (!validPassword) {
            printf(
                "Password must be at least 8 characters long and contain "
                "uppercase, lowercase and numbers\n");
        }
    } while (!validPassword);

    do {
        printf("Please enter your email: ");
        fgets(newUser.email, sizeof(newUser.email), stdin);
        if (strchr(newUser.email, '\n') == NULL) wipeBufferOut();
        newUser.email[strcspn(newUser.email, "\n")] = '\0';

        validEmail = validateEmail(newUser.email);
        if (!validEmail) printf("Email is invalid\n");
    } while (!validEmail);

    do {
        printf("Please enter your phone number: ");
        fgets(newUser.phone, sizeof(newUser.phone), stdin);
        if (strchr(newUser.phone, '\n') == NULL) wipeBufferOut();
        newUser.phone[strcspn(newUser.phone, "\n")] = '\0';

        validPhone = validatePhone(newUser.phone);
        if (!validPhone) printf("Phone number is invalid\n");
    } while (!validPhone);
    newUser.status = 1;
    newUser.loginFailedCount = 0;
    strcpy(newUser.role, "user");
    newUser.lockedTime = 0;

    insert(list, newUser);
    printf("Successfully created your new account\n");
    saveToFile(newUser);
}

void changePassword(List **list) {
    if (currentUser == NULL) {
        printf("Please login first\n");
        return;
    }

    char oldPassword[50], newPassword[50];

    do {
        printf("Please enter old password: ");
        fgets(oldPassword, sizeof(oldPassword), stdin);
        if (strchr(oldPassword, '\n') == NULL) wipeBufferOut();
        oldPassword[strcspn(oldPassword, "\n")] = '\0';

        if (strcmp(currentUser->password, oldPassword) != 0)
            printf("Wrong password\n");

    } while (strcmp(currentUser->password, oldPassword) != 0);

    int validNewPassword;
    do {
        printf("Please enter new password: ");
        fgets(newPassword, sizeof(newPassword), stdin);
        if (strchr(newPassword, '\n') == NULL) wipeBufferOut();
        newPassword[strcspn(newPassword, "\n")] = '\0';

        validNewPassword = validatePassword(newPassword);
        if (!validNewPassword) {
            printf(
                "Password must be at least 8 characters long and contain both "
                "letters and numbers. Please try again.\n");
        }
    } while (!validNewPassword);

    strcpy(currentUser->password, newPassword);
    saveAllData(*list);
    printf("Password changed successfully\n");
}

void updateAccountInfo(List **list) {
    if (currentUser == NULL) {
        printf("Please login first\n");
        return;
    }

    char newEmail[50], newPhone[20];
    int validEmail = 0, validPhone = 0;

    do {
        printf("Please enter new email (or enter 'q' to keep current): ");
        fgets(newEmail, sizeof(newEmail), stdin);
        if (strchr(newEmail, '\n') == NULL) wipeBufferOut();
        newEmail[strcspn(newEmail, "\n")] = '\0';

        if (strcmp(newEmail, "q") == 0)
            validEmail = 1;
        else if (validateEmail(newEmail)) {
            strcpy(currentUser->email, newEmail);
            validEmail = 1;
        } else
            printf("Email is invalid\n");

    } while (!validEmail);

    do {
        printf(
            "Please enter new phone number (or enter 'q' to keep current): ");
        fgets(newPhone, sizeof(newPhone), stdin);
        if (strchr(newPhone, '\n') == NULL) wipeBufferOut();
        newPhone[strcspn(newPhone, "\n")] = '\0';

        if (strcmp(newPhone, "q") == 0)
            validPhone = 1;
        else if (validatePhone(newPhone)) {
            strcpy(currentUser->phone, newPhone);
            validPhone = 1;
        } else
            printf("Phone number is invalid\n");

    } while (!validPhone);

    saveAllData(*list);
    printf("Account info updated successfully\n");
}

void resetPassword(List **list) {
    char username[50], email[50];
    List *userNode = NULL;

    int validUsername, validEmail, validNewPassword;
    do {
        printf("Please enter your username: ");
        fgets(username, sizeof(username), stdin);
        if (strchr(username, '\n') == NULL) wipeBufferOut();
        username[strcspn(username, "\n")] = '\0';

        validUsername = validateUsername(username);
        if (!validUsername) {
            printf("Username is invalid\n");
            continue;
        }

        userNode = checkExist(list, username);
        if (userNode == NULL) printf("Account not exist\n");
    } while (!validUsername || userNode == NULL);

    do {
        printf("Please enter your registered email: ");
        fgets(email, sizeof(email), stdin);
        if (strchr(email, '\n') == NULL) wipeBufferOut();
        email[strcspn(email, "\n")] = '\0';

        validEmail = validateEmail(email);
        if (!validEmail) {
            printf("Email is invalid\n");
            continue;
        }

        if (strcmp(userNode->userData.email, email) != 0)
            printf("Email does not match our records\n");

    } while (!validEmail || strcmp(userNode->userData.email, email) != 0);

    char newPassword[50];
    do {
        printf("Please enter your new password: ");
        fgets(newPassword, sizeof(newPassword), stdin);
        if (strchr(newPassword, '\n') == NULL) wipeBufferOut();
        newPassword[strcspn(newPassword, "\n")] = '\0';

        validNewPassword = validatePassword(newPassword);
        if (!validNewPassword) {
            printf(
                "Password must be at least 8 characters long and contain both "
                "letters and numbers. Please try again.\n");
        }
    } while (!validNewPassword);

    strcpy(userNode->userData.password, newPassword);
    userNode->userData.loginFailedCount = 0;
    userNode->userData.status = 1;
    saveAllData(*list);
    printf("Password reset successfully\n");
}
