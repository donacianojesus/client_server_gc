#include "network.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

int setup_server_socket(const char *ip, int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        return -1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (strcmp(ip, "0.0.0.0") == 0 || strcmp(ip, "localhost") == 0) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        server_addr.sin_addr.s_addr = inet_addr(ip);
    }
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return -1;
    }
    
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        return -1;
    }
    
    printf("Server listening on %s:%d\n", ip, port);
    return server_socket;
}

int accept_client_connection(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Accept failed");
        return -1;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("New client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
    
    return client_socket;
}

int receive_message(int client_socket, message_t *message) {
    int bytes_received = recv(client_socket, message, sizeof(message_t), 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Client disconnected\n");
        } else {
            perror("Recv failed");
        }
        return -1;
    }
    return bytes_received;
}

int send_message(int client_socket, const message_t *message) {
    int bytes_sent = send(client_socket, message, sizeof(message_t), 0);
    if (bytes_sent < 0) {
        perror("Send failed");
        return -1;
    }
    return bytes_sent;
}

void add_client(int client_socket, list_t *users) {
    // This will be called after successful authentication
    // The actual user will be added when they log in
}

void remove_client(int client_socket, list_t *users) {
    user_list_remove_by_socket(users, client_socket);
    close(client_socket);
}

void broadcast_to_all_clients(const message_t *message, list_t *users) {
    list_node_t *current = users->head;
    while (current) {
        user_t *user = (user_t*)current->data;
        if (user->is_online) {
            send_message(user->socket_fd, message);
        }
        current = current->next;
    }
}

void handle_client_message(int client_socket, message_t *message, list_t *users, list_t *groups) {
    switch (message->type) {
        case MSG_LOGIN:
            process_login_message(client_socket, message, users);
            break;
        case MSG_REGISTER:
            process_register_message(client_socket, message, users);
            break;
        case MSG_JOIN_GROUP:
            process_join_group_message(client_socket, message, users, groups);
            break;
        case MSG_CREATE_GROUP:
            process_create_group_message(client_socket, message, users, groups);
            break;
        case MSG_CHAT_MESSAGE:
            process_chat_message(client_socket, message, users, groups);
            break;
        case MSG_LEAVE_GROUP:
            process_leave_group_message(client_socket, message, users, groups);
            break;
        case MSG_LOGOUT:
            remove_client(client_socket, users);
            break;
        default:
            printf("Unknown message type: %d\n", message->type);
            break;
    }
}

void process_login_message(int client_socket, const message_t *message, list_t *users) {
    auth_message_t *auth_msg = (auth_message_t*)message->data;
    response_message_t response;
    
    if (authenticate_user(auth_msg->username, auth_msg->password)) {
        // Check if user is already online
        user_t *existing_user = user_list_find_by_username(users, auth_msg->username);
        if (existing_user && existing_user->is_online) {
            response.success = 0;
            strcpy(response.message, "User already logged in");
        } else {
            // Create new user or update existing one
            user_t *user;
            if (existing_user) {
                user = existing_user;
                user->socket_fd = client_socket;
                user->is_online = 1;
            } else {
                user = create_user(auth_msg->username, client_socket);
                list_append(users, user);
            }
            
            response.success = 1;
            strcpy(response.message, "Login successful");
            printf("User %s logged in\n", auth_msg->username);
        }
    } else {
        response.success = 0;
        strcpy(response.message, "Invalid username or password");
    }
    
    message_t response_msg;
    response_msg.type = MSG_LOGIN_RESPONSE;
    response_msg.length = sizeof(response_message_t);
    memcpy(response_msg.data, &response, sizeof(response_message_t));
    
    send_message(client_socket, &response_msg);
}

void process_register_message(int client_socket, const message_t *message, list_t *users) {
    auth_message_t *auth_msg = (auth_message_t*)message->data;
    response_message_t response;
    
    if (register_user(auth_msg->username, auth_msg->password)) {
        response.success = 1;
        strcpy(response.message, "Registration successful");
        printf("New user registered: %s\n", auth_msg->username);
    } else {
        response.success = 0;
        strcpy(response.message, "Username already exists");
    }
    
    message_t response_msg;
    response_msg.type = MSG_REGISTER_RESPONSE;
    response_msg.length = sizeof(response_message_t);
    memcpy(response_msg.data, &response, sizeof(response_message_t));
    
    send_message(client_socket, &response_msg);
}

void process_join_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups) {
    group_message_t *group_msg = (group_message_t*)message->data;
    response_message_t response;
    
    user_t *user = user_list_find_by_socket(users, client_socket);
    if (!user) {
        response.success = 0;
        strcpy(response.message, "User not authenticated");
    } else {
        group_t *group = group_list_find_by_name(groups, group_msg->group_name);
        if (!group) {
            response.success = 0;
            strcpy(response.message, "Group does not exist");
        } else {
            if (add_user_to_group(user, group_msg->group_name)) {
                add_member_to_group(group, user->username);
                response.success = 1;
                strcpy(response.message, "Successfully joined group");
                printf("User %s joined group %s\n", user->username, group_msg->group_name);
            } else {
                response.success = 0;
                strcpy(response.message, "Failed to join group");
            }
        }
    }
    
    message_t response_msg;
    response_msg.type = MSG_GROUP_RESPONSE;
    response_msg.length = sizeof(response_message_t);
    memcpy(response_msg.data, &response, sizeof(response_message_t));
    
    send_message(client_socket, &response_msg);
}

void process_create_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups) {
    group_message_t *group_msg = (group_message_t*)message->data;
    response_message_t response;
    
    user_t *user = user_list_find_by_socket(users, client_socket);
    if (!user) {
        response.success = 0;
        strcpy(response.message, "User not authenticated");
    } else {
        // Check if group already exists
        group_t *existing_group = group_list_find_by_name(groups, group_msg->group_name);
        if (existing_group) {
            response.success = 0;
            strcpy(response.message, "Group already exists");
        } else {
            group_t *new_group = create_group(group_msg->group_name);
            if (new_group) {
                list_append(groups, new_group);
                add_user_to_group(user, group_msg->group_name);
                add_member_to_group(new_group, user->username);
                response.success = 1;
                strcpy(response.message, "Group created successfully");
                printf("Group %s created by user %s\n", group_msg->group_name, user->username);
            } else {
                response.success = 0;
                strcpy(response.message, "Failed to create group");
            }
        }
    }
    
    message_t response_msg;
    response_msg.type = MSG_GROUP_RESPONSE;
    response_msg.length = sizeof(response_message_t);
    memcpy(response_msg.data, &response, sizeof(response_message_t));
    
    send_message(client_socket, &response_msg);
}

void process_chat_message(int client_socket, const message_t *message, list_t *users, list_t *groups) {
    chat_message_t *chat_msg = (chat_message_t*)message->data;
    
    user_t *user = user_list_find_by_socket(users, client_socket);
    if (!user) return;
    
    // Verify user is in the group
    if (!is_user_in_group(user, group_list_find_by_name(groups, chat_msg->group_name))) {
        return;
    }
    
    // Broadcast message to group
    broadcast_message_to_group(chat_msg->group_name, chat_msg->message, user->username, users);
    printf("Message from %s in group %s: %s\n", user->username, chat_msg->group_name, chat_msg->message);
}

void process_leave_group_message(int client_socket, const message_t *message, list_t *users, list_t *groups) {
    group_message_t *group_msg = (group_message_t*)message->data;
    response_message_t response;
    
    user_t *user = user_list_find_by_socket(users, client_socket);
    if (!user) {
        response.success = 0;
        strcpy(response.message, "User not authenticated");
    } else {
        group_t *group = group_list_find_by_name(groups, group_msg->group_name);
        if (!group) {
            response.success = 0;
            strcpy(response.message, "Group does not exist");
        } else {
            if (remove_user_from_group(user, group_msg->group_name)) {
                remove_member_from_group(group, user->username);
                response.success = 1;
                strcpy(response.message, "Successfully left group");
                printf("User %s left group %s\n", user->username, group_msg->group_name);
            } else {
                response.success = 0;
                strcpy(response.message, "Failed to leave group");
            }
        }
    }
    
    message_t response_msg;
    response_msg.type = MSG_GROUP_RESPONSE;
    response_msg.length = sizeof(response_message_t);
    memcpy(response_msg.data, &response, sizeof(response_message_t));
    
    send_message(client_socket, &response_msg);
}
