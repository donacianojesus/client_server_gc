#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "../common/protocol.h"
#include "../common/list.h"

// Network setup functions
int setup_server_socket(const char *ip, int port);
int accept_client_connection(int server_socket);

// Message handling functions
int receive_message(int client_socket, message_t *message);
int send_message(int client_socket, const message_t *message);
void handle_client_message(int client_socket, message_t *message, list_t *users, list_t *groups);

// Client management functions
void add_client(int client_socket, list_t *users);
void remove_client(int client_socket, list_t *users);
void broadcast_to_all_clients(const message_t *message, list_t *users);

// Message processing functions
void process_login_message(int client_socket, const message_t *message, list_t *users);
void process_register_message(int client_socket, const message_t *message, list_t *users);
void process_join_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups);
void process_create_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups);
void process_chat_message(int client_socket, const message_t *message, list_t *users, list_t *groups);
void process_leave_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups);

#endif // SERVER_NETWORK_H
