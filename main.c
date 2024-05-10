#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

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
    int snapshotFile = open("Snapshot.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(snapshotFile == -1){
        perror("Unable to create/open Snapshot file");
        closedir(dir);
        exit(EXIT_FAILURE);
    }

    struct dirent* entry;
    while((entry = readdir(dir))!=NULL){
        // Skip , and ..
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0)
            continue;

        // Get metadata
        Metadata metadata;
        strcpy(metadata.name, entry->d_name);

        write(snapshotFile, metadata.name, strlen(metadata.name));
        write(snapshotFile, "\n", 1);

        closedir(dir);
        close(snapshotFile);
        
    }


}


int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    return 0;
}