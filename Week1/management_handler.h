#pragma once

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fileprocess.h"
#include "linklist.h"
#include "validation.h"

extern Data *currentUser;

void wipeBufferOut();
void newUserRegister(List **list);
void login(List **list);
void changePassword(List **list);
void updateAccountInfo(List **list);
void resetPassword(List **list);
void viewLoginHistory();
void authorization(List **list);
void signOut();
int checkAccountStatus(List **list);