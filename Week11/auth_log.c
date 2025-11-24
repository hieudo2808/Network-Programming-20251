#include <stdio.h>
#include <time.h>
#include <string.h>
#include "auth_log.h"

void log_auth(const char* event_type, const char* username, const char* ip, int port, const char* result) {
    FILE* log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    if (strcmp(event_type, "ACCOUNT_LOCKED") == 0) {
        fprintf(log_file, "[%s] ACCOUNT_LOCKED %s\n", timestamp, username);
    } else if (strcmp(event_type, "LOGOUT") == 0) {
        fprintf(log_file, "[%s] LOGOUT %s from %s:%d\n", timestamp, username, ip, port);
    } else {
        // LOGIN event
        fprintf(log_file, "[%s] LOGIN %s from %s:%d %s\n", timestamp, username, ip, port, result);
    }

    fclose(log_file);
}
