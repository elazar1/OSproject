#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct {
    char name[256];
} Metadata;

typedef struct Node{
    Metadata data;
    struct Node* next;    
} Node;

// Function to capture initial snapshot
void captureSnapshot(const char* directory){
    // Open the directory
    DIR* dir = opendir(directory);
    if(dir == NULL){
        perror("Unable to open the directory.");
        exit(EXIT_FAILURE);
    }

    // Create/open Snapshot.txt for writing in outputDir
    char snapshotFilePath[512];
    

}


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    return 0;
}