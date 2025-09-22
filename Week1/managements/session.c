#include "../management_handler.h"

Data *currentUser = NULL;

int checkAccountStatus(List **list) {
    if ((*list)->userData.status == 0) {
        time_t currentTime = time(NULL);
        double secondsPassed =
            difftime(currentTime, (*list)->userData.lockedTime);
        if (secondsPassed > 600) {  // 10 mins
            (*list)->userData.status = 1;
            (*list)->userData.lockedTime = 0;
            saveAllData(*list);
            return 1;
        } else
            return 0;
    }
    return 1;
}

void login(List **list) {
    char username[50], password[50];
    int validUsername, validPassword;
    List *userNode = NULL;

    do {
        printf("Please enter username: ");
        fgets(username, sizeof(username), stdin);
        if (strchr(username, '\n') == NULL) wipeBufferOut();
        username[strcspn(username, "\n")] = '\0';

        validUsername = validateUsername(username);
        if (!validUsername)
            printf("Username is invalid\n");
        else {
            userNode = checkExist(list, username);
            if (userNode == NULL) printf("Cannot find account\n");
        }

    } while (!validUsername || userNode == NULL);

    int attempts = 0, loginSuccess = 0;

    while (attempts < 3 && !loginSuccess) {
        do {
            printf("Please enter password: ");
            fgets(password, sizeof(password), stdin);
            if (strchr(password, '\n') == NULL) wipeBufferOut();
            password[strcspn(password, "\n")] = '\0';

            validPassword = validatePassword(password);
            if (!validPassword) {
                printf("Password is invalid\n");
            }
        } while (!validPassword);

        if (strcmp(userNode->userData.password, password) == 0) {
            loginSuccess = 1;
        } else {
            attempts++;
            userNode->userData.loginFailedCount++;
            printf("Wrong password\n");

            if (attempts >= 3) {
                userNode->userData.status = 0;
                userNode->userData.lockedTime = time(NULL);
                saveAllData(*list);
                printf("Your account is blocked\n");
                return;
            }
        }
    }

    if (userNode->userData.status == 0) {
        if (checkAccountStatus(&userNode) == 0) {
            printf("Your account is blocked\n");
            return;
        }
    }

    userNode->userData.loginFailedCount = 0;
    currentUser = &userNode->userData;

    saveHistory(currentUser->username);
    printf("Welcome %s\n", currentUser->username);
}

void signOut() {
    if (currentUser == NULL) {
        printf("You're not logged in\n");
        return;
    }

    currentUser = NULL;
    printf("Goodbye\n");
}