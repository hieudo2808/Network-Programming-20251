#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct data {
    char username[50];
    char password[50];
    char email[100];
    char homepage[200];
    int status;  // 0: blocked, 1: active
} Data;

typedef struct linklist {
    Data userData;
    struct linklist *next;
} List;

void insert(List **list, Data userData);
void deleteNode(List **list, char username[]);
void freeAllList(List *list);
List *checkExist(List **list, char username[]);