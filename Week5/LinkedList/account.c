#include "account.h"

List *createNode(Data newData) {
    List *newNode = (List *)malloc(sizeof(List));
    if (newNode == NULL) {
        printf("Cant create a new node");
        exit(1);
    }

    strcpy(newNode->userData.username, newData.username);
    strcpy(newNode->userData.password, newData.password);
    strcpy(newNode->userData.email, newData.email);
    strcpy(newNode->userData.homepage, newData.homepage);
    newNode->userData.status = newData.status;
    newNode->next = NULL;

    return newNode;
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
    if (checkExist(list, userData.username) != NULL) {
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

void freeAllList(List *list) {
    if (list == NULL) return;

    freeAllList(list->next);
    free(list);
}