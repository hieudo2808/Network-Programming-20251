#pragma once

#define LOG_FILE "auth.log"
void log_auth(const char* event_type, const char* username, const char* ip, int port, const char* result);
