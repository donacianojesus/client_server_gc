#ifndef CLIENT_AUTH_H
#define CLIENT_AUTH_H

#include "../common/protocol.h"

// Client authentication functions
int client_login(int server_socket, const char *username, const char *password);
int client_register(int server_socket, const char *username, const char *password);
void handle_auth_response(const message_t *message);

// Client state
extern int is_authenticated;
extern char current_username[MAX_USERNAME_LEN];

#endif // CLIENT_AUTH_H
