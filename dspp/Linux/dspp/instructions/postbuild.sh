#!/bin/bash

# Enable script debugging (if needed)
#set -x

# Set the source, build, and third-party directories from arguments
SOURCE_DIR="$1"
BUILD_DIR="$2"
TPTY_DIR="$3"

# Create the target directories if they don't exist
mkdir -p "$BUILD_DIR/dspp/include"
mkdir -p "$BUILD_DIR/dspp/libs"

# Copy .h files to the include folder
echo "Copying .h files to $BUILD_DIR/dspp/include..."
for f in "$SOURCE_DIR"/*.h; do
    cp "$f" "$BUILD_DIR/dspp/include/"
done

# Move 3rd Party files to the include folder
echo "Copying 3rd Party includes..."
find "$TPTY_DIR" -type d -name "include" | while read dir; do
    echo "Copying $dir to $BUILD_DIR/include/dspp..."
    
    PARENT_NAME=$(basename "$(dirname "$dir")")
    mkdir -p "$BUILD_DIR/dspp/include/$PARENT_NAME/include"
    
    # Copy the files
    cp -r "$dir"/* "$BUILD_DIR/dspp/include/$PARENT_NAME/include/"
done

# Move .a files to the libs folder
echo "Moving .a files to $BUILD_DIR/libs..."
find "$BUILD_DIR" -type f -name "*.a" | while read file; do
    mv -f "$file" "$BUILD_DIR/dspp/libs/"
done

echo "Post-build process complete."
