#include <stdio.h>
#include <time.h>
#include "auth_log.h"

void log_auth(const char* event, const char* user, const char* ip, int port, const char* result) {
    FILE* f = fopen(LOG_FILE, "a");
    if (!f) return;

    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    if (port > 0)
        fprintf(f, "[%s] %s %s from %s:%d %s\n", ts, event, user, ip, port, result);
    else
        fprintf(f, "[%s] %s %s\n", ts, event, user);

    fclose(f);
}
