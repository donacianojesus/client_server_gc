#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64
#define MAX_GROUP_NAME_LEN 32
#define MAX_MESSAGE_LEN 1024
#define MAX_GROUPS_PER_USER 10
#define MAX_USERS_PER_GROUP 20
#define MAX_CLIENTS 100

// Message types
typedef enum {
    MSG_LOGIN = 1,
    MSG_REGISTER = 2,
    MSG_LOGIN_RESPONSE = 3,
    MSG_REGISTER_RESPONSE = 4,
    MSG_JOIN_GROUP = 5,
    MSG_CREATE_GROUP = 6,
    MSG_GROUP_RESPONSE = 7,
    MSG_CHAT_MESSAGE = 8,
    MSG_LEAVE_GROUP = 9,
    MSG_LOGOUT = 10,
    MSG_ERROR = 11,
    MSG_SUCCESS = 12
} message_type_t;

// Message structure
typedef struct {
    message_type_t type;
    uint32_t length;
    char data[MAX_MESSAGE_LEN];
} message_t;

// Login/Register message
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} auth_message_t;

// Response message
typedef struct {
    int success;
    char message[MAX_MESSAGE_LEN];
} response_message_t;

// Group message
typedef struct {
    char group_name[MAX_GROUP_NAME_LEN];
    char username[MAX_USERNAME_LEN];
} group_message_t;

// Chat message
typedef struct {
    char group_name[MAX_GROUP_NAME_LEN];
    char username[MAX_USERNAME_LEN];
    char message[MAX_MESSAGE_LEN];
    time_t timestamp;
} chat_message_t;

// User structure
typedef struct {
    char username[MAX_USERNAME_LEN];
    int socket_fd;
    int is_online;
    char groups[MAX_GROUPS_PER_USER][MAX_GROUP_NAME_LEN];
    int group_count;
} user_t;

// Group structure
typedef struct {
    char name[MAX_GROUP_NAME_LEN];
    char members[MAX_USERS_PER_GROUP][MAX_USERNAME_LEN];
    int member_count;
} group_t;

#endif // PROTOCOL_H
