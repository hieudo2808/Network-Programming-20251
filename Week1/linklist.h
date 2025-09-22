#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct data {
    char username[50];
    char password[50];
    char email[50];
    char phone[20];
    int status;  // 0: blocked, 1: active
    int loginFailedCount;
    char role[10];
    time_t lockedTime;
} Data;

typedef struct linklist {
    Data userData;
    struct linklist *next;
} List;

void insert(List **list, Data userData);
void deleteNode(List **list, char username[]);
void printList(List *list);
void freeAllList(List *list);
List *checkExist(List **list, char username[]);