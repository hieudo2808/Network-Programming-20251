#include "serverHandler.h"

#include <stdio.h>
#include <string.h>

#include "../FileHandler/fileprocess.h"
#include "../Validation/validation.h"

void loginHandler(SessionData *sd, List *accounts, char *buffer,
                  char *response) {
    List *userNode = checkExist(&accounts, sd->tempUsername);

    if (!userNode) {
        strcpy(response, "Not OK");
        sd->tempUsername[0] = '\0';
        sd->loginFailedCount = 0;
    } else if (userNode->userData.status == 0) {
        strcpy(response, "Account not ready");
        sd->tempUsername[0] = '\0';
        sd->loginFailedCount = 0;
    } else if (strcmp(userNode->userData.password, buffer) == 0) {
        strcpy(response, "OK");
        sd->currentUser = &(userNode->userData);
        sd->tempUsername[0] = '\0';
        sd->loginFailedCount = 0;
        printf("User %s logged in\n", sd->currentUser->username);
    } else {
        sd->loginFailedCount++;
        if (sd->loginFailedCount >= 3) {
            userNode->userData.status = 0;
            saveAllData(accounts);
            strcpy(response, "Account not ready");
            printf("Account %s blocked\n", sd->tempUsername);
            sd->tempUsername[0] = '\0';
            sd->loginFailedCount = 0;
        } else
            strcpy(response, "Not OK");
    }
}

void authenticatedUserHandler(SessionData *sd, char *buffer, char *response,
                              List *accounts) {
    if (strcmp(buffer, "homepage") == 0) {
        strcpy(response, sd->currentUser->homepage);
        printf("Sent homepage to %s\n", sd->currentUser->username);
    } else {
        if (!isAlphanumeric(buffer))
            strcpy(response, "Error");
        else {
            char letters[50], digits[50];
            encodePassword(buffer, letters, digits);
            strcpy(sd->currentUser->password, buffer);
            saveAllData(accounts);
            snprintf(response, 1024, "%s\n%s", letters, digits);
            printf("User %s changed password\n", sd->currentUser->username);
        }
    }
}
