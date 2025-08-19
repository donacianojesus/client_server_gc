#!/bin/bash

echo "=== TCP Group Chat Application Build Test ==="
echo "Testing build process..."

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "❌ Make is not installed. Please install build-essential (Ubuntu/Debian) or Development Tools (CentOS/RHEL)"
    exit 1
fi

# Check if gcc is available
if ! command -v gcc &> /dev/null; then
    echo "❌ GCC is not installed. Please install build-essential (Ubuntu/Debian) or Development Tools (CentOS/RHEL)"
    exit 1
fi

echo "✅ Build tools found"

# Clean any previous builds
echo "🧹 Cleaning previous builds..."
make clean

# Build the application
echo "🔨 Building application..."
if make; then
    echo "✅ Build successful!"
    
    # Check if executables were created
    if [ -f "target/server" ] && [ -f "target/client" ]; then
        echo "✅ Executables created successfully"
        echo "   - Server: target/server"
        echo "   - Client: target/client"
        
        # Show file sizes
        echo "📊 File sizes:"
        ls -lh target/
        
        echo ""
        echo "🎉 Build test completed successfully!"
        echo ""
        echo "To run the application:"
        echo "1. Start server: ./target/server 0.0.0.0 8080"
        echo "2. Start client: ./target/client 127.0.0.1 8080"
        echo ""
        echo "Or use Docker Compose:"
        echo "1. Start server: docker compose run --rm --name server server"
        echo "2. Start client: docker compose run --rm --name client1 client1"
        
    else
        echo "❌ Executables not found after build"
        exit 1
    fi
else
    echo "❌ Build failed!"
    exit 1
fi
