#ifndef SERVER_AUTH_H
#define SERVER_AUTH_H

#include "../common/protocol.h"
#include "../common/list.h"

// User authentication functions
int authenticate_user(const char *username, const char *password);
int register_user(const char *username, const char *password);
int add_user_to_group(user_t *user, const char *group_name);
int remove_user_from_group(user_t *user, const char *group_name);
int is_user_in_group(user_t *user, const char *group_name);
void broadcast_message_to_group(const char *group_name, const char *message, const char *sender, list_t *users);

// User management functions
user_t* create_user(const char *username, int socket_fd);
void destroy_user(user_t *user);
int save_user_data(const char *username, const char *password);
int load_user_data(const char *username, char *password);

// Group management functions
group_t* create_group(const char *group_name);
void destroy_group(group_t *group);
int add_member_to_group(group_t *group, const char *username);
int remove_member_from_group(group_t *group, const char *username);

#endif // SERVER_AUTH_H
