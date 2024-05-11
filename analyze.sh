#!/bin/bash

# Function to check for non-ASCII characters and key words
check_for_suspicious_characters() {
    local file="$1"
    local keywords=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
    local suspicious=0

    for keyword in "${keywords[@]}"; do
        if grep -q "$keyword" "$file"; then
            suspicious=1
            break
        fi
    done

    if [[ $suspicious -eq 1 ]]; then
        echo 1
    else
        echo 0
    fi

}

# Function to analyze the file
analyze_file() {
    local file="$1"
    local num_lines=$(wc -l < "$file")
    local num_words=$(wc -w < "$file")
    local num_chars=$(wc -m < "$file")

    if [[ $num_lines -lt 1 && $num_words -gt 1000 && $num_chars -gt 2000 ]]; then
        echo "$file"
    elif [ $(check_for_suspicious_characters "$file") -eq 1 ]; then
        echo "$file"
    else
        echo "SAFE"
    fi
}


# Main script
if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <file_to_analyze> <directory_for_malicious_files>"
    exit 1
fi

file_to_analyze="$1"
directory_for_malicious_files="$2"

if [[ ! -f $file_to_analyze ]]; then
    echo "Error: $file_to_analyze is not a regular file."
    exit 1
fi

result=$(analyze_file "$file_to_analyze")

if [[ $result != "SAFE" ]]; then
    mv "$file_to_analyze" "$directory_for_malicious_files"
    echo "$result"
else
    echo "$result"
fi