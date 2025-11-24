#ifndef AUTH_LOG_H
#define AUTH_LOG_H

#define LOG_FILE "auth.log"

// Log authentication events
void log_auth(const char* event_type, const char* username, const char* ip, int port, const char* result);

#endif
