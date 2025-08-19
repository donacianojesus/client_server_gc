CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lpthread

# Directories
SERVER_DIR = server
CLIENT_DIR = client
COMMON_DIR = common
TARGET_DIR = target

# Source files
SERVER_SOURCES = $(SERVER_DIR)/server.c $(SERVER_DIR)/network.c $(SERVER_DIR)/auth.c
CLIENT_SOURCES = $(CLIENT_DIR)/client.c $(CLIENT_DIR)/network.c $(CLIENT_DIR)/auth.c
COMMON_SOURCES = $(COMMON_DIR)/list.c

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)

# Executables
SERVER_EXEC = $(TARGET_DIR)/server
CLIENT_EXEC = $(TARGET_DIR)/client

# Default target
all: $(TARGET_DIR) $(SERVER_EXEC) $(CLIENT_EXEC)

# Create target directory
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# Server executable
$(SERVER_EXEC): $(SERVER_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Client executable
$(CLIENT_EXEC): $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile server source files
$(SERVER_DIR)/%.o: $(SERVER_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile client source files
$(CLIENT_DIR)/%.o: $(CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile common source files
$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)
	rm -rf $(TARGET_DIR)

# Install dependencies (for Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential gdb

# Install dependencies (for CentOS/RHEL/Fedora)
install-deps-rpm:
	sudo yum groupinstall -y "Development Tools"
	sudo yum install -y gdb

# Run server locally
run-server: $(SERVER_EXEC)
	$(SERVER_EXEC) 0.0.0.0 8080

# Run client locally
run-client: $(CLIENT_EXEC)
	$(CLIENT_EXEC) 127.0.0.1 8080

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: all

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: all

# Check for memory leaks with valgrind
memcheck: all
	valgrind --leak-check=full --show-leak-kinds=all $(SERVER_EXEC) 0.0.0.0 8080

# Format code (requires clang-format)
format:
	clang-format -i $(SERVER_DIR)/*.c $(SERVER_DIR)/*.h
	clang-format -i $(CLIENT_DIR)/*.c $(CLIENT_DIR)/*.h
	clang-format -i $(COMMON_DIR)/*.c $(COMMON_DIR)/*.h

# Static analysis with cppcheck
analyze:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SERVER_DIR) $(CLIENT_DIR) $(COMMON_DIR)

.PHONY: all clean install-deps install-deps-rpm run-server run-client debug release memcheck format analyze
