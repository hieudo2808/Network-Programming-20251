#define PCRE2_CODE_UNIT_WIDTH 8

#include "validation.h"

#include <ctype.h>
#include <pcre2.h>
#include <stdio.h>

int matchRegex(const char *string, const char *pattern) {
    int errornumber;
    size_t erroroffset;
    pcre2_code *re;
    pcre2_match_data *match_data;
    int rc;

    // Compile regex
    re =
        pcre2_compile((PCRE2_SPTR)pattern,    // pattern
                      PCRE2_ZERO_TERMINATED,  // pattern length (NUL-terminated)
                      0,                      // default options
                      &errornumber,           // error code
                      &erroroffset,           // error offset
                      NULL);                  // compile context

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %d: %s\n",
                (int)erroroffset, buffer);
        return 0;
    }

    // Create match data
    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    // Run match
    rc = pcre2_match(re,                  // the compiled pattern
                     (PCRE2_SPTR)string,  // subject string
                     strlen(string),      // length of subject
                     0,                   // start offset
                     0,                   // options
                     match_data,          // match data block
                     NULL);               // match context

    // Free resources
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return rc >= 0;  // match found if rc >= 0
}

int validateUsername(const char *username) {
    if (username == NULL) return 0;

    return matchRegex(username, "^[A-Za-z][A-Za-z0-9_]{4,49}$");
}

int validatePassword(const char *password) {
    if (password == NULL) return 0;

    return matchRegex(password,
                      "^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)[A-Za-z\\d!@#$%%^&*()_+"
                      "\\-=\\[\\]{};':\\\"\\\\|,.<>/?]{8,}$");
}

int validateEmail(const char *email) {
    if (email == NULL) return 0;

    const char emailPattern[] =
        "(?:[a-z0-9!#$%%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%%&'*+/=?^_`{|}~-]+)*"
        "|\"(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21\\x23-\\x5b\\x5d-\\x7f]"
        "|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])*\")@"
        "(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9]"
        "(?:[a-z0-9-]*[a-z0-9])?|\\[(?:(?:25[0-5]|2[0-4][0-9]"
        "|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]"
        "|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:"
        "(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f"
        "\\x21-\\x5a\\x53-\\x7f]|\\\\[\\x01-\\x09"
        "\\x0b\\x0c\\x0e-\\x7f])+))";

    return matchRegex(email, emailPattern);
}

int validatePhone(const char *phone) {
    if (phone == NULL) return 0;

    return matchRegex(phone, "^(0[0-9]{9,10}|\\+84[0-9]{9,10})$");
}

// -lpcre2-8