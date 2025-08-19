#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define USERS_FILE "users.dat"
#define MAX_LINE_LEN 256

// Simple file-based user storage
static int save_user_data(const char *username, const char *password) {
    FILE *file = fopen(USERS_FILE, "a");
    if (!file) return 0;
    
    fprintf(file, "%s:%s\n", username, password);
    fclose(file);
    return 1;
}

static int load_user_data(const char *username, char *password) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) return 0;
    
    char line[MAX_LINE_LEN];
    char stored_username[MAX_USERNAME_LEN];
    char stored_password[MAX_PASSWORD_LEN];
    
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^:]:%s", stored_username, stored_password) == 2) {
            if (strcmp(stored_username, username) == 0) {
                strcpy(password, stored_password);
                fclose(file);
                return 1;
            }
        }
    }
    
    fclose(file);
    return 0;
}

int authenticate_user(const char *username, const char *password) {
    char stored_password[MAX_PASSWORD_LEN];
    
    if (load_user_data(username, stored_password)) {
        return strcmp(password, stored_password) == 0;
    }
    return 0;
}

int register_user(const char *username, const char *password) {
    // Check if user already exists
    char stored_password[MAX_PASSWORD_LEN];
    if (load_user_data(username, stored_password)) {
        return 0; // User already exists
    }
    
    return save_user_data(username, password);
}

user_t* create_user(const char *username, int socket_fd) {
    user_t *user = malloc(sizeof(user_t));
    if (!user) return NULL;
    
    strncpy(user->username, username, MAX_USERNAME_LEN - 1);
    user->username[MAX_USERNAME_LEN - 1] = '\0';
    user->socket_fd = socket_fd;
    user->is_online = 1;
    user->group_count = 0;
    
    // Initialize groups array
    for (int i = 0; i < MAX_GROUPS_PER_USER; i++) {
        user->groups[i][0] = '\0';
    }
    
    return user;
}

void destroy_user(user_t *user) {
    if (user) {
        free(user);
    }
}

int add_user_to_group(user_t *user, const char *group_name) {
    if (!user || !group_name || user->group_count >= MAX_GROUPS_PER_USER) {
        return 0;
    }
    
    // Check if user is already in the group
    if (is_user_in_group(user, group_name)) {
        return 0;
    }
    
    strncpy(user->groups[user->group_count], group_name, MAX_GROUP_NAME_LEN - 1);
    user->groups[user->group_count][MAX_GROUP_NAME_LEN - 1] = '\0';
    user->group_count++;
    
    return 1;
}

int remove_user_from_group(user_t *user, const char *group_name) {
    if (!user || !group_name) return 0;
    
    for (int i = 0; i < user->group_count; i++) {
        if (strcmp(user->groups[i], group_name) == 0) {
            // Shift remaining groups
            for (int j = i; j < user->group_count - 1; j++) {
                strcpy(user->groups[j], user->groups[j + 1]);
            }
            user->group_count--;
            return 1;
        }
    }
    return 0;
}

int is_user_in_group(user_t *user, const char *group_name) {
    if (!user || !group_name) return 0;
    
    for (int i = 0; i < user->group_count; i++) {
        if (strcmp(user->groups[i], group_name) == 0) {
            return 1;
        }
    }
    return 0;
}

group_t* create_group(const char *group_name) {
    group_t *group = malloc(sizeof(group_t));
    if (!group) return NULL;
    
    strncpy(group->name, group_name, MAX_GROUP_NAME_LEN - 1);
    group->name[MAX_GROUP_NAME_LEN - 1] = '\0';
    group->member_count = 0;
    
    // Initialize members array
    for (int i = 0; i < MAX_USERS_PER_GROUP; i++) {
        group->members[i][0] = '\0';
    }
    
    return group;
}

void destroy_group(group_t *group) {
    if (group) {
        free(group);
    }
}

int add_member_to_group(group_t *group, const char *username) {
    if (!group || !username || group->member_count >= MAX_USERS_PER_GROUP) {
        return 0;
    }
    
    // Check if user is already a member
    for (int i = 0; i < group->member_count; i++) {
        if (strcmp(group->members[i], username) == 0) {
            return 0;
        }
    }
    
    strncpy(group->members[group->member_count], username, MAX_USERNAME_LEN - 1);
    group->members[group->member_count][MAX_USERNAME_LEN - 1] = '\0';
    group->member_count++;
    
    return 1;
}

int remove_member_from_group(group_t *group, const char *username) {
    if (!group || !username) return 0;
    
    for (int i = 0; i < group->member_count; i++) {
        if (strcmp(group->members[i], username) == 0) {
            // Shift remaining members
            for (int j = i; j < group->member_count - 1; j++) {
                strcpy(group->members[j], group->members[j + 1]);
            }
            group->member_count--;
            return 1;
        }
    }
    return 0;
}

void broadcast_message_to_group(const char *group_name, const char *message, const char *sender, list_t *users) {
    if (!group_name || !message || !sender || !users) return;
    
    chat_message_t chat_msg;
    strncpy(chat_msg.group_name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(chat_msg.username, sender, MAX_USERNAME_LEN - 1);
    strncpy(chat_msg.message, message, MAX_MESSAGE_LEN - 1);
    chat_msg.timestamp = time(NULL);
    
    // Create the message to send
    message_t msg;
    msg.type = MSG_CHAT_MESSAGE;
    msg.length = sizeof(chat_message_t);
    memcpy(msg.data, &chat_msg, sizeof(chat_message_t));
    
    // Send to all online users in the group
    list_node_t *current = users->head;
    while (current) {
        user_t *user = (user_t*)current->data;
        if (user->is_online && is_user_in_group(user, group_name)) {
            // Send message to user
            send(user->socket_fd, &msg, sizeof(message_t), 0);
        }
        current = current->next;
    }
}
