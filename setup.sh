#!/bin/bash

# Check if the user provided an argument
if [ $# -eq 0 ]; then
    echo "Usage: $0 c (create) / r (remove)"
    exit 1
fi

# Loop through directories 1 to 9
for i in {1..9}; do
    if [ "$1" = "c" ]; then
        # Create directories and files
        mkdir -p "../dir$i/subdir1" "../dir$i/subdir2/subdir1"
        echo "ceva1" > "../dir$i/file.txt"
        echo "ceva1" > "../dir$i/subdir1/file.txt"
        echo "ceva2" > "../dir$i/subdir2/subdir1/file1.txt"
        echo "ceva3" > "../dir$i/subdir2/file.txt"
        echo "ceva4" > "../dir$i/subdir2/subdir1/file2.txt"
    elif [ "$1" = "r" ]; then
        # Remove directories
        rm -rf "../dir$i"
    else
        echo "Usage: $0 c (create) / r (remove)"
        exit 1
    fi
done
