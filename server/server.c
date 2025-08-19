#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "network.h"
#include "auth.h"
#include "../common/list.h"

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

static int server_socket = -1;
static list_t *users = NULL;
static list_t *groups = NULL;

void cleanup() {
    printf("\nShutting down server...\n");
    
    if (users) {
        user_list_destroy(users);
    }
    if (groups) {
        group_list_destroy(groups);
    }
    if (server_socket != -1) {
        close(server_socket);
    }
    
    exit(0);
}

void signal_handler(int sig) {
    cleanup();
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port_number>\n", argv[0]);
        printf("Example: %s 0.0.0.0 8080\n", argv[0]);
        return 1;
    }
    
    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    
    if (port <= 0 || port > 65535) {
        printf("Invalid port number. Must be between 1 and 65535.\n");
        return 1;
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize data structures
    users = user_list_create();
    groups = group_list_create();
    
    if (!users || !groups) {
        printf("Failed to initialize data structures\n");
        cleanup();
        return 1;
    }
    
    // Set up server socket
    server_socket = setup_server_socket(server_ip, port);
    if (server_socket == -1) {
        printf("Failed to set up server socket\n");
        cleanup();
        return 1;
    }
    
    printf("TCP Group Chat Server started successfully!\n");
    printf("Server IP: %s, Port: %d\n", server_ip, port);
    printf("Press Ctrl+C to stop the server\n\n");
    
    fd_set read_fds;
    int max_fd = server_socket;
    
    // Main server loop
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        
        // Add all client sockets to the set
        list_node_t *current = users->head;
        while (current) {
            user_t *user = (user_t*)current->data;
            if (user->is_online && user->socket_fd > 0) {
                FD_SET(user->socket_fd, &read_fds);
                if (user->socket_fd > max_fd) {
                    max_fd = user->socket_fd;
                }
            }
            current = current->next;
        }
        
        // Wait for activity on any socket
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by signal
            }
            perror("Select failed");
            break;
        }
        
        // Check for new connections
        if (FD_ISSET(server_socket, &read_fds)) {
            int client_socket = accept_client_connection(server_socket);
            if (client_socket > 0) {
                if (client_socket > max_fd) {
                    max_fd = client_socket;
                }
                printf("New client connection accepted (socket: %d)\n", client_socket);
            }
        }
        
        // Check for data from existing clients
        current = users->head;
        while (current) {
            user_t *user = (user_t*)current->data;
            list_node_t *next = current->next; // Store next before potential removal
            
            if (user->is_online && user->socket_fd > 0 && FD_ISSET(user->socket_fd, &read_fds)) {
                message_t message;
                int bytes_received = receive_message(user->socket_fd, &message);
                
                if (bytes_received <= 0) {
                    // Client disconnected
                    printf("Client %s disconnected\n", user->username);
                    remove_client(user->socket_fd, users);
                } else {
                    // Process the message
                    handle_client_message(user->socket_fd, &message, users, groups);
                }
            }
            
            current = next;
        }
    }
    
    cleanup();
    return 0;
}
