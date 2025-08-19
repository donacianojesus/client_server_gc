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
#include <time.h>

int connect_to_server(const char *server_ip, int port) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Failed to create socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Invalid server IP address\n");
        close(client_socket);
        return -1;
    }
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        return -1;
    }
    
    printf("Connected to server %s:%d\n", server_ip, port);
    return client_socket;
}

void disconnect_from_server(int server_socket) {
    if (server_socket > 0) {
        close(server_socket);
    }
}

int send_message(int server_socket, const message_t *message) {
    int bytes_sent = send(server_socket, message, sizeof(message_t), 0);
    if (bytes_sent < 0) {
        perror("Send failed");
        return -1;
    }
    return bytes_sent;
}

int receive_message(int server_socket, message_t *message) {
    int bytes_received = recv(server_socket, message, sizeof(message_t), 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Server disconnected\n");
        } else {
            perror("Recv failed");
        }
        return -1;
    }
    return bytes_received;
}

int join_group(int server_socket, const char *group_name) {
    if (!group_name) return 0;
    
    group_message_t group_msg;
    strncpy(group_msg.group_name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(group_msg.username, current_username, MAX_USERNAME_LEN - 1);
    
    message_t message;
    message.type = MSG_JOIN_GROUP;
    message.length = sizeof(group_message_t);
    memcpy(message.data, &group_msg, sizeof(group_message_t));
    
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send join group message\n");
        return 0;
    }
    
    // Wait for response
    message_t response;
    if (receive_message(server_socket, &response) < 0) {
        return 0;
    }
    
    if (response.type == MSG_GROUP_RESPONSE) {
        response_message_t *resp = (response_message_t*)response.data;
        if (resp->success) {
            printf("✓ %s\n", resp->message);
            return 1;
        } else {
            printf("✗ %s\n", resp->message);
            return 0;
        }
    }
    
    return 0;
}

int create_group(int server_socket, const char *group_name) {
    if (!group_name) return 0;
    
    group_message_t group_msg;
    strncpy(group_msg.group_name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(group_msg.username, current_username, MAX_USERNAME_LEN - 1);
    
    message_t message;
    message.type = MSG_CREATE_GROUP;
    message.length = sizeof(group_message_t);
    memcpy(message.data, &group_msg, sizeof(group_message_t));
    
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send create group message\n");
        return 0;
    }
    
    // Wait for response
    message_t response;
    if (receive_message(server_socket, &response) < 0) {
        return 0;
    }
    
    if (response.type == MSG_GROUP_RESPONSE) {
        response_message_t *resp = (response_message_t*)response.data;
        if (resp->success) {
            printf("✓ %s\n", resp->message);
            return 1;
        } else {
            printf("✗ %s\n", resp->message);
            return 0;
        }
    }
    
    return 0;
}

int send_chat_message(int server_socket, const char *group_name, const char *message_text) {
    if (!group_name || !message_text) return 0;
    
    chat_message_t chat_msg;
    strncpy(chat_msg.group_name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(chat_msg.username, current_username, MAX_USERNAME_LEN - 1);
    strncpy(chat_msg.message, message_text, MAX_MESSAGE_LEN - 1);
    chat_msg.timestamp = time(NULL);
    
    message_t message;
    message.type = MSG_CHAT_MESSAGE;
    message.length = sizeof(chat_message_t);
    memcpy(message.data, &chat_msg, sizeof(chat_message_t));
    
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send chat message\n");
        return 0;
    }
    
    return 1;
}

int leave_group(int server_socket, const char *group_name) {
    if (!group_name) return 0;
    
    group_message_t group_msg;
    strncpy(group_msg.group_name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(group_msg.username, current_username, MAX_USERNAME_LEN - 1);
    
    message_t message;
    message.type = MSG_LEAVE_GROUP;
    message.length = sizeof(group_message_t);
    memcpy(message.data, &group_msg, sizeof(group_message_t));
    
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send leave group message\n");
        return 0;
    }
    
    // Wait for response
    message_t response;
    if (receive_message(server_socket, &response) < 0) {
        return 0;
    }
    
    if (response.type == MSG_GROUP_RESPONSE) {
        response_message_t *resp = (response_message_t*)response.data;
        if (resp->success) {
            printf("✓ %s\n", resp->message);
            return 1;
        } else {
            printf("✗ %s\n", resp->message);
            return 0;
        }
    }
    
    return 0;
}

void logout(int server_socket) {
    message_t message;
    message.type = MSG_LOGOUT;
    message.length = 0;
    
    send_message(server_socket, &message);
    is_authenticated = 0;
    current_username[0] = '\0';
}

void handle_server_message(const message_t *message) {
    switch (message->type) {
        case MSG_CHAT_MESSAGE: {
            chat_message_t *chat_msg = (chat_message_t*)message->data;
            time_t timestamp = chat_msg->timestamp;
            char time_str[26];
            ctime_r(&timestamp, time_str);
            time_str[24] = '\0'; // Remove newline
            
            printf("[%s] %s in %s: %s\n", 
                   time_str, chat_msg->username, chat_msg->group_name, chat_msg->message);
            break;
        }
        case MSG_LOGIN_RESPONSE:
        case MSG_REGISTER_RESPONSE:
            handle_auth_response(message);
            break;
        case MSG_GROUP_RESPONSE: {
            response_message_t *resp = (response_message_t*)message->data;
            if (resp->success) {
                printf("✓ %s\n", resp->message);
            } else {
                printf("✗ %s\n", resp->message);
            }
            break;
        }
        default:
            printf("Received unknown message type: %d\n", message->type);
            break;
    }
}
