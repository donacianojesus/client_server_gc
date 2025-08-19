# TCP Group Chat Application

A robust TCP-based group chat system with client-server architecture implemented in C. This application supports multiple clients, user authentication, group management, and secure messaging. Users can join groups and send messages, which are only delivered to online members of the intended group.

## Features

- **User Authentication**: Secure login and registration system
- **Group Chat**: Create or join groups and send messages to group members
- **Message Delivery**: Messages are delivered only to online members of the group
- **Cross-Platform**: Can run locally or on AWS using Docker
- **Real-time Communication**: Non-blocking I/O with select() for efficient client handling
- **Persistent User Data**: File-based user storage for authentication

## Project Structure

```
.
├── client/                 # Client application
│   ├── auth.c             # Client-side authentication logic
│   ├── auth.h             # Header for authentication module
│   ├── client.c           # Main client application logic
│   ├── network.c          # Handles network communication for client
│   └── network.h          # Header for client network module
├── common/                 # Shared components
│   ├── list.c             # Utility functions for managing lists
│   ├── list.h             # Header for list utility
│   └── protocol.h         # Common protocol definitions
├── server/                 # Server application
│   ├── auth.c             # Handles server-side authentication logic
│   ├── auth.h             # Header for server authentication module
│   ├── network.c          # Handles network communication for server
│   ├── network.h          # Header for server network module
│   └── server.c           # Main server application logic
├── target/                 # Output directory for compiled binaries
├── compose.yaml            # Docker Compose configuration
├── Dockerfile              # Dockerfile for building containers
├── Makefile                # Makefile for compiling the project
└── README.md               # This file
```

## Prerequisites

- **Docker** installation
- **Basic knowledge of TCP networking**
- **Access to an AWS server** (if deploying remotely)
- **GCC compiler** (for local compilation)

## Quick Start

### Running Locally with Docker

1. **Start the Server**
   ```bash
   docker compose run --rm --name server server
   ```

2. **Start Multiple Clients**
   ```bash
   # Start client 1
   docker compose run --rm --name client1 client1
   
   # Start client 2
   docker compose run --rm --name client2 client2
   
   # Start client 3
   docker compose run --rm --name client3 client3
   ```

### Running Locally without Docker

1. **Compile the Project**
   ```bash
   make clean
   make
   ```

2. **Start the Server**
   ```bash
   ./target/server <server_ip> <port_number>
   # Example: ./target/server 0.0.0.0 8080
   ```

3. **Start the Client**
   ```bash
   ./target/client <server_ip> <port_number>
   # Example: ./target/client 127.0.0.1 8080
   ```

## Usage

### Client Commands

Once connected to the server, clients can use the following commands:

- `login <username> <password>` - Login to the server
- `register <username> <password>` - Register a new account
- `create <group_name>` - Create a new group
- `join <group_name>` - Join an existing group
- `leave <group_name>` - Leave a group
- `send <group_name> <message>` - Send a message to a group
- `groups` - List your groups
- `logout` - Logout from the server
- `quit` - Exit the client
- `help` - Show available commands

### Example Session

```
=== TCP Group Chat Client ===
Available commands:
  login <username> <password>  - Login to the server
  register <username> <password> - Register a new account
  create <group_name>          - Create a new group
  join <group_name>            - Join an existing group
  leave <group_name>           - Leave a group
  send <group_name> <message>  - Send a message to a group
  groups                        - List your groups
  logout                        - Logout from the server
  quit                          - Exit the client
  help                          - Show this help
================================

register name password123
✓ Registration successful! You can now login.

login name password123
✓ Login successful
Welcome, name!

create general
✓ Group created successfully

send general Hello everyone!
Message sent to group general

[2024-01-15 10:30:45] bob in general: Hi name!
```

## AWS Deployment

### 1. Transfer Files to AWS Server

```bash
scp -r ./server ./client ./common Makefile firstname-lastname@ip:/usr/home/firstname-lastname/
```

### 2. SSH into AWS Server

```bash
ssh firstname-lastname@ip
```

### 3. Compile the Project

```bash
make clean
make
cd target
```

### 4. Start the Server

```bash
./server <server_ip> <port_number>
# Example: ./server 0.0.0.0 8080
```

### 5. Start Remote Clients

```bash
docker compose run --rm --name remoteclient1 remoteclient
docker compose run --rm --name remoteclient2 remoteclient
```

## Building and Development

### Makefile Targets

- `make` - Build both server and client
- `make clean` - Remove build artifacts
- `make debug` - Build with debug symbols
- `make release` - Build with optimization
- `make run-server` - Run server locally on port 8080
- `make run-client` - Run client locally connecting to 127.0.0.1:8080

### Development Tools

- **GDB**: Debugging support
- **Valgrind**: Memory leak detection
- **cppcheck**: Static code analysis
- **clang-format**: Code formatting

### Code Quality

```bash
# Format code
make format

# Static analysis
make analyze

# Memory leak check
make memcheck
```

## Protocol Details

### Message Types

- `MSG_LOGIN` (1) - User login request
- `MSG_REGISTER` (2) - User registration request
- `MSG_LOGIN_RESPONSE` (3) - Login response
- `MSG_REGISTER_RESPONSE` (4) - Registration response
- `MSG_JOIN_GROUP` (5) - Join group request
- `MSG_CREATE_GROUP` (6) - Create group request
- `MSG_GROUP_RESPONSE` (7) - Group operation response
- `MSG_CHAT_MESSAGE` (8) - Chat message
- `MSG_LEAVE_GROUP` (9) - Leave group request
- `MSG_LOGOUT` (10) - User logout
- `MSG_ERROR` (11) - Error message
- `MSG_SUCCESS` (12) - Success message

### Message Structure

```c
typedef struct {
    message_type_t type;
    uint32_t length;
    char data[MAX_MESSAGE_LEN];
} message_t;
```

## Security Features

- **Password-based authentication**
- **User session management**
- **Group membership validation**
- **Message delivery only to group members**

## Performance Features

- **Non-blocking I/O with select()**
- **Efficient client management**
- **Memory-efficient data structures**
- **Scalable architecture for multiple clients**

## Troubleshooting

### Common Issues

1. **Port already in use**
   - Change the port number in the server command
   - Check if another service is using the port

2. **Connection refused**
   - Ensure the server is running
   - Check firewall settings
   - Verify IP address and port

3. **Compilation errors**
   - Ensure GCC is installed
   - Check for missing dependencies
   - Verify C99 standard support

### Debug Mode

```bash
make debug
gdb ./target/server
```

