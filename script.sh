#!/bin/bash

for i in {1..9}
do
    if [ "$1" = "c" ]; then
        mkdir "../dir$i"
        mkdir "../dir$i/subdir1"
        mkdir "../dir$i/subdir2"
        mkdir "../dir$i/subdir2/subdir1"
        openssl rand 200 > "../dir$i/file.txt"
        openssl rand 200 > "../dir$i/subdir1/file.txt"
        openssl rand 200 > "../dir$i/subdir2/subdir1/file1.txt"
        openssl rand 200 > "../dir$i/subdir2/file.txt"
        openssl rand 200 > "../dir$i/subdir2/subdir1/file2.txt"
    elif [ "$1" = "a" ]; then
        openssl rand 200 >> "../dir$i/file.txt"
        openssl rand 200 >> "../dir$i/subdir1/file.txt"
        openssl rand 200 >> "../dir$i/subdir2/subdir1/file1.txt"
    elif [ "$1" = "r" ]; then
        rm -rf "../dir$i"
    elif [ "$1" = "o" ]; then
        rm -rf "../result"
    else 
        echo "./script.sh c (create) / r (remove)";
        exit 1
    fi
done