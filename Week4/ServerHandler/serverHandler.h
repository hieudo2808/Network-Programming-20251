#pragma once

#include "../LinkedList/session.h"
#include "../LinkedList/account.h"

void loginHandler(SessionData *sd, List *accounts, char *buffer, char *response);
void authenticatedUserHandler(SessionData *sd, char *buffer, char *response, List *accounts);
