#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "linklist.h"

void saveToFile(Data dataToSave) {
    FILE *file = fopen("account.txt", "a+");
    if (!file) {
        printf("Error while opening account file\n");
        return;
    }

    fprintf(file, "%s %s %s %s %d %s %ld\n", dataToSave.username,
            dataToSave.password, dataToSave.email, dataToSave.phone,
            dataToSave.status, dataToSave.role, (long)dataToSave.lockedTime);

    fclose(file);
}

void saveAllData(List *list) {
    FILE *file = fopen("account.txt", "w");
    if (!file) {
        printf("Error while opening account file\n");
        return;
    }

    List *current = list;
    while (current != NULL) {
        fprintf(file, "%s %s %s %s %d %s %ld\n", current->userData.username,
                current->userData.password, current->userData.email,
                current->userData.phone, current->userData.status,
                current->userData.role, (long)current->userData.lockedTime);
        current = current->next;
    }

    fclose(file);
}

void loadData(List **list) {
    FILE *file = fopen("account.txt", "r");
    if (!file) {
        printf("Error while opening account file\n");
        return;
    }

    Data tempData;
    long lockedTime;
    while (fscanf(file, "%49s %49s %49s %19s %d %9s %ld\n", tempData.username,
                  tempData.password, tempData.email, tempData.phone,
                  &tempData.status, tempData.role, &lockedTime) == 7) {
        tempData.lockedTime = (time_t)lockedTime;
        tempData.loginFailedCount = 0;
        insert(list, tempData);
    }

    fclose(file);
}

void saveHistory(char username[]) {
    FILE *file = fopen("history.txt", "a+");
    if (!file) {
        printf("Error while opening history file\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    fprintf(file, "%s | %02d/%02d/%04d | %02d:%02d:%02d\n", username,
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
            local->tm_hour, local->tm_min, local->tm_sec);

    fclose(file);
}

void loadHistory(char username[]) {
    FILE *file = fopen("history.txt", "r");
    if (!file) {
        printf("Error while opening history file\n");
        return;
    }

    char record[256];
    printf("Login History:\n");
    while (fgets(record, sizeof(record), file)) {
        if (strstr(record, username) != NULL) {
            printf("%s", record);
        }
    }
    printf("\n");

    fclose(file);
}