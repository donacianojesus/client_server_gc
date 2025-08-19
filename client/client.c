#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "auth.h"
#include "network.h"

#define BUFFER_SIZE 1024
#define MAX_INPUT 256

static int server_socket = -1;
static int running = 1;

void cleanup() {
    printf("\nDisconnecting from server...\n");
    if (is_authenticated) {
        logout(server_socket);
    }
    disconnect_from_server(server_socket);
    exit(0);
}

void signal_handler(int sig) {
    cleanup();
}

void print_help() {
    printf("\n=== TCP Group Chat Client ===\n");
    printf("Available commands:\n");
    printf("  login <username> <password>  - Login to the server\n");
    printf("  register <username> <password> - Register a new account\n");
    printf("  create <group_name>          - Create a new group\n");
    printf("  join <group_name>            - Join an existing group\n");
    printf("  leave <group_name>           - Leave a group\n");
    printf("  send <group_name> <message>  - Send a message to a group\n");
    printf("  groups                        - List your groups\n");
    printf("  logout                        - Logout from the server\n");
    printf("  quit                          - Exit the client\n");
    printf("  help                          - Show this help\n");
    printf("================================\n\n");
}

void print_groups() {
    // This would require additional server support to list user's groups
    printf("Groups functionality not yet implemented\n");
}

void handle_user_input(const char *input) {
    char command[MAX_INPUT];
    char arg1[MAX_INPUT];
    char arg2[MAX_INPUT];
    char message[MAX_INPUT];
    
    if (sscanf(input, "%s %s %s", command, arg1, arg2) >= 2) {
        if (strcmp(command, "login") == 0) {
            if (is_authenticated) {
                printf("Already logged in as %s\n", current_username);
                return;
            }
            
            char password[MAX_INPUT];
            printf("Enter password: ");
            if (fgets(password, sizeof(password), stdin)) {
                password[strcspn(password, "\n")] = 0;
                if (client_login(server_socket, arg1, password)) {
                    printf("Welcome, %s!\n", current_username);
                }
            }
        }
        else if (strcmp(command, "register") == 0) {
            char password[MAX_INPUT];
            printf("Enter password: ");
            if (fgets(password, sizeof(password), stdin)) {
                password[strcspn(password, "\n")] = 0;
                if (client_register(server_socket, arg1, password)) {
                    printf("Registration successful! You can now login.\n");
                }
            }
        }
        else if (strcmp(command, "create") == 0) {
            if (!is_authenticated) {
                printf("Please login first\n");
                return;
            }
            create_group(server_socket, arg1);
        }
        else if (strcmp(command, "join") == 0) {
            if (!is_authenticated) {
                printf("Please login first\n");
                return;
            }
            join_group(server_socket, arg1);
        }
        else if (strcmp(command, "leave") == 0) {
            if (!is_authenticated) {
                printf("Please login first\n");
                return;
            }
            leave_group(server_socket, arg1);
        }
        else if (strcmp(command, "send") == 0) {
            if (!is_authenticated) {
                printf("Please login first\n");
                return;
            }
            
            // Get the message part (everything after group name)
            char *msg_start = strstr(input, arg1) + strlen(arg1);
            while (*msg_start == ' ') msg_start++;
            
            if (strlen(msg_start) > 0) {
                send_chat_message(server_socket, arg1, msg_start);
                printf("Message sent to group %s\n", arg1);
            } else {
                printf("Please provide a message to send\n");
            }
        }
        else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }
    else if (sscanf(input, "%s", command) == 1) {
        if (strcmp(command, "groups") == 0) {
            if (!is_authenticated) {
                printf("Please login first\n");
                return;
            }
            print_groups();
        }
        else if (strcmp(command, "logout") == 0) {
            if (!is_authenticated) {
                printf("Not logged in\n");
                return;
            }
            logout(server_socket);
            printf("Logged out successfully\n");
        }
        else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            cleanup();
        }
        else if (strcmp(command, "help") == 0) {
            print_help();
        }
        else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port_number>\n", argv[0]);
        printf("Example: %s 127.0.0.1 8080\n", argv[0]);
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
    
    // Connect to server
    server_socket = connect_to_server(server_ip, port);
    if (server_socket == -1) {
        printf("Failed to connect to server\n");
        return 1;
    }
    
    printf("Connected to TCP Group Chat Server!\n");
    print_help();
    
    // Set up for non-blocking input
    fd_set read_fds;
    char input_buffer[MAX_INPUT];
    
    // Main client loop
    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(server_socket, &read_fds);
        
        int max_fd = (server_socket > STDIN_FILENO) ? server_socket : STDIN_FILENO;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by signal
            }
            perror("Select failed");
            break;
        }
        
        // Check for server messages
        if (FD_ISSET(server_socket, &read_fds)) {
            message_t message;
            if (receive_message(server_socket, &message) > 0) {
                handle_server_message(&message);
            } else {
                printf("Server disconnected\n");
                break;
            }
        }
        
        // Check for user input
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(input_buffer, sizeof(input_buffer), stdin)) {
                input_buffer[strcspn(input_buffer, "\n")] = 0; // Remove newline
                
                if (strlen(input_buffer) > 0) {
                    handle_user_input(input_buffer);
                }
            }
        }
    }
    
    cleanup();
    return 0;
}
