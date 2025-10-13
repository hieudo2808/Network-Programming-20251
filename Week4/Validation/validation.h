#pragma once

#include <ctype.h>
#include <string.h>

int isAlphanumeric(char password[]);
void encodePassword(char password[], char *letters, char *digits);