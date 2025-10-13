#pragma once

#include <ctype.h>
#include <string.h>

int isAlphanumeric(const char *password);
void encodePassword(const char *password, char *letters, char *digits);