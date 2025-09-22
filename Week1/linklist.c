#include "linklist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List *createNode(Data newData) {
    List *newNode = (List *)malloc(sizeof(List));
    if (newNode == NULL) {
        printf("Cant create a new node");
        exit(1);
    }

    strcpy(newNode->userData.email, newData.email);
    strcpy(newNode->userData.password, newData.password);
    strcpy(newNode->userData.phone, newData.phone);
    strcpy(newNode->userData.username, newData.username);
    newNode->userData.status = 1;
    newNode->userData.loginFailedCount = 0;
    strcpy(newNode->userData.role, "user");
    newNode->userData.lockedTime = 0;
    newNode->next = NULL;

    return newNode;
}

int checkValidInfo(Data dataToCheck) {
    if (dataToCheck.status != 0 && dataToCheck.status != 1) {
        return 0;
    }

    if (strcmp(dataToCheck.role, "user") != 0 &&
        strcmp(dataToCheck.role, "admin") != 0) {
        return 0;
    }

    return 1;
}

List *checkExist(List **list, char username[]) {
    List *temp = *list;
    while (temp != NULL) {
        if (strcmp(temp->userData.username, username) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}

void insert(List **list, Data userData) {
    if (checkExist(list, userData.username) != NULL ||
        !checkValidInfo(userData)) {
        printf("Username already exists\n");
        return;
    }

    List *newNode = createNode(userData);

    if (*list == NULL) {
        *list = newNode;
        return;
    }

    List *temp = *list;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

void deleteNode(List **list, char username[]) {
    if (*list == NULL) return;

    List *temp = *list, *prev = NULL;

    while (temp != NULL && strcmp(temp->userData.username, username) != 0) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return;

    if (prev == NULL) {
        *list = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);
}

void printList(List *list) {
    while (list != NULL) {
        printf("%s %s %s %s %d\n", list->userData.username,
               list->userData.password, list->userData.email,
               list->userData.phone, list->userData.status);
        list = list->next;
    }
}

void freeAllList(List *list) {
    if (list == NULL) return;

    freeAllList(list->next);
    free(list);
}