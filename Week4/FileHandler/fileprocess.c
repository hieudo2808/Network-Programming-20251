#include "fileprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void saveToFile(Data dataToSave) {
    FILE *file = fopen("account.txt", "a+");
    if (!file) {
        printf("Error while opening account file\n");
        return;
    }

    fprintf(file, "%s %s %s %s %d\n", dataToSave.username,
            dataToSave.password, dataToSave.email, dataToSave.homepage,
            dataToSave.status);

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
        fprintf(file, "%s %s %s %s %d\n", current->userData.username,
                current->userData.password, current->userData.email,
                current->userData.homepage, current->userData.status);
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
    while (fscanf(file, "%49s %49s %99s %199s %d\n", tempData.username,
                  tempData.password, tempData.email, tempData.homepage,
                  &tempData.status) == 5) {
        insert(list, tempData);
    }

    fclose(file);
}