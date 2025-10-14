#include "validation.h"

int isAlphanumeric(char password[]) {
    if (password == NULL || strlen(password) == 0) return 0;

    for (int i = 0; i < strlen(password); i++) {
        if (!isalnum(password[i])) return 0;
    }
    return 1;
}

void encodePassword(char password[], char *letters, char *digits) {
    int letterIdx = 0, digitIdx = 0;

    for (int i = 0; i < strlen(password); i++) {
        if (isalpha(password[i]))
            letters[letterIdx++] = password[i];
        else if (isdigit(password[i]))
            digits[digitIdx++] = password[i];
    }
    letters[letterIdx] = '\0';
    digits[digitIdx] = '\0';
}