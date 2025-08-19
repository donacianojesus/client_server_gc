#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include "../common/protocol.h"

// Network connection functions
int connect_to_server(const char *server_ip, int port);
void disconnect_from_server(int server_socket);

// Message handling functions
int send_message(int server_socket, const message_t *message);
int receive_message(int server_socket, message_t *message);

// Chat functions
int join_group(int server_socket, const char *group_name);
int create_group(int server_socket, const char *group_name);
int send_chat_message(int server_socket, const char *group_name, const char *message);
int leave_group(int server_socket, const char *group_name);
void logout(int server_socket);

// Message processing functions
void handle_server_message(const message_t *message);

#endif // CLIENT_NETWORK_H
