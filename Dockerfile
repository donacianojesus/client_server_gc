# Use Ubuntu 22.04 as base image
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    valgrind \
    cppcheck \
    clang-format \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN make clean && make

# Create a non-root user
RUN useradd -m -s /bin/bash chatuser && \
    chown -R chatuser:chatuser /app

# Switch to non-root user
USER chatuser

# Expose default port
EXPOSE 8080

# Default command (can be overridden)
CMD ["/bin/bash"]
