#include "auth.h"
#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Client state
int is_authenticated = 0;
char current_username[MAX_USERNAME_LEN] = "";

int client_login(int server_socket, const char *username, const char *password) {
    if (!username || !password) return 0;
    
    // Create login message
    auth_message_t auth_msg;
    strncpy(auth_msg.username, username, MAX_USERNAME_LEN - 1);
    strncpy(auth_msg.password, password, MAX_PASSWORD_LEN - 1);
    
    message_t message;
    message.type = MSG_LOGIN;
    message.length = sizeof(auth_message_t);
    memcpy(message.data, &auth_msg, sizeof(auth_message_t));
    
    // Send login message
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send login message\n");
        return 0;
    }
    
    // Wait for response
    message_t response;
    if (receive_message(server_socket, &response) < 0) {
        printf("Failed to receive login response\n");
        return 0;
    }
    
    if (response.type == MSG_LOGIN_RESPONSE) {
        handle_auth_response(&response);
        if (is_authenticated) {
            strncpy(current_username, username, MAX_USERNAME_LEN - 1);
            current_username[MAX_USERNAME_LEN - 1] = '\0';
        }
        return is_authenticated;
    }
    
    return 0;
}

int client_register(int server_socket, const char *username, const char *password) {
    if (!username || !password) return 0;
    
    // Create register message
    auth_message_t auth_msg;
    strncpy(auth_msg.username, username, MAX_USERNAME_LEN - 1);
    strncpy(auth_msg.password, password, MAX_PASSWORD_LEN - 1);
    
    message_t message;
    message.type = MSG_REGISTER;
    message.length = sizeof(auth_message_t);
    memcpy(message.data, &auth_msg, sizeof(auth_message_t));
    
    // Send register message
    if (send_message(server_socket, &message) < 0) {
        printf("Failed to send register message\n");
        return 0;
    }
    
    // Wait for response
    message_t response;
    if (receive_message(server_socket, &response) < 0) {
        printf("Failed to receive register response\n");
        return 0;
    }
    
    if (response.type == MSG_REGISTER_RESPONSE) {
        handle_auth_response(&response);
        return response->success;
    }
    
    return 0;
}

void handle_auth_response(const message_t *message) {
    if (message->type == MSG_LOGIN_RESPONSE || message->type == MSG_REGISTER_RESPONSE) {
        response_message_t *response = (response_message_t*)message->data;
        
        if (response->success) {
            is_authenticated = 1;
            printf("✓ %s\n", response->message);
        } else {
            is_authenticated = 0;
            printf("✗ %s\n", response->message);
        }
    }
}
