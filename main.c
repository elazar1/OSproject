#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

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

void monitorChanges(const char* directory){
    // Open Snapshot.txt for reading

    int snapshotFile = open("Snapshot.txt", O_RDWR);
    if(snapshotFile == -1){
        perror("Unable to open Snapshot.txt");
        exit(EXIT_FAILURE);
    }

    DIR* dir = opendir(directory);
    if(dir == NULL){
        perror("Unable to open directory");
        fclose(snapshotFile);
        exit(EXIT_FAILURE);
    }

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
        // Skip , and ..
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0)
            continue;

    
    char entryName[256];
    strcpy(entryName, entry->d_name);
    if(!findEntryInSnapshot(snapshotFile, entryName)){
        printf("New file: %s\n", entryName);
        // Update Snapshot.txt
        write(snapshotFile, entryName, strlen(entryName));
        write(snapshotFile, "\n", 1);
        fsync(snapshotFile);
        }
    } 
}

// Helper function to find an entry in the snapshot file
int findEntryInSnapshot(int snapshotFile, const char* entryName) {
    lseek(snapshotFile, 0, SEEK_SET); // Move file pointer to the beginning

    char buffer[256];
    ssize_t bytesRead;
    while ((bytesRead = read(snapshotFile, buffer, sizeof(buffer))) > 0) {
        char* token = strtok(buffer, "\n");
        while (token != NULL) {
            if (strcmp(token, entryName) == 0)
                return 1; // Entry found
            token = strtok(NULL, "\n");
        }
    }

    return 0; // Entry not found
}

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    char* directory = argv[1];


    captureSnapshot(directory);

    monitorChanges(directory);

    return 0;
}