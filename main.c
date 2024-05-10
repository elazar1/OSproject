#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAX_DIRECTORIES 10

// Define structures for metadata and directory entry

typedef struct {
    char name[256]; // Name of file or directory
    time_t last_modified; // Last modified timestamp
    mode_t permissions; // File permissions
    // Add other metadata fields as needed
} Metadata;

// Function prototypes
void captureSnapshot(const char* directory, const char* outputDir);
void monitorChanges(const char* directory, const char* outputDir);
int findEntryInSnapshot(int snapshotFile, const char* entryName);

// Function to capture initial snapshot
void captureSnapshot(const char* directory, const char* outputDir) {
    // Open the directory
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        perror("Unable to open directory");
        exit(EXIT_FAILURE);
    }

    // Create/open Snapshot.txt for writing in outputDir
    char snapshotFilePath[512];
    snprintf(snapshotFilePath, sizeof(snapshotFilePath), "%s/Snapshot.txt", outputDir);
    int snapshotFile = open(snapshotFilePath, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (snapshotFile == -1) {
        perror("Unable to create/open Snapshot.txt");
        closedir(dir);
        exit(EXIT_FAILURE);
    }

    // Traverse directory
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Get metadata
        Metadata metadata;
        strcpy(metadata.name, entry->d_name);
        // Get last modified time and permissions
        char entryPath[512];
        snprintf(entryPath, sizeof(entryPath), "%s/%s", directory, entry->d_name);
        struct stat st;
        if (stat(entryPath, &st) == -1) {
            perror("Unable to get file stats");
            close(snapshotFile);
            closedir(dir);
            exit(EXIT_FAILURE);
        }
        metadata.last_modified = st.st_mtime;
        metadata.permissions = st.st_mode;

        // Write metadata to Snapshot.txt
        char buffer[1024];
        int len = snprintf(buffer, sizeof(buffer), "Entry: %s\n", metadata.name);
        write(snapshotFile, buffer, len);
        len = snprintf(buffer, sizeof(buffer), "Last Modified: %s", ctime(&metadata.last_modified));
        write(snapshotFile, buffer, len);
        len = snprintf(buffer, sizeof(buffer), "Permissions: %o\n", metadata.permissions);
        write(snapshotFile, buffer, len);
        write(snapshotFile, "\n", 1);

        // If entry is a directory, recursively capture snapshot
        if (S_ISDIR(st.st_mode)) {
            captureSnapshot(entryPath, outputDir);
        }
    }

    // Close directory and file
    closedir(dir);
    close(snapshotFile);
}

// Function to monitor changes
void monitorChanges(const char* directory, const char* outputDir) {
    // Open Snapshot.txt for reading from outputDir
    char snapshotFilePath[512];
    snprintf(snapshotFilePath, sizeof(snapshotFilePath), "%s/Snapshot.txt", outputDir);
    int snapshotFile = open(snapshotFilePath, O_RDONLY);
    if (snapshotFile == -1) {
        perror("Unable to open Snapshot.txt");
        exit(EXIT_FAILURE);
    }

    // Open the directory
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        perror("Unable to open directory");
        close(snapshotFile);
        exit(EXIT_FAILURE);
    }

    // Traverse directory
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Check if entry exists in Snapshot.txt
        // If not, it's a new file
        char entryName[256];
        strcpy(entryName, entry->d_name);
        if (!findEntryInSnapshot(snapshotFile, entryName)) {
            printf("New file: %s\n", entryName);
            // Update Snapshot.txt
            char buffer[256];
            int len = snprintf(buffer, sizeof(buffer), "Entry: %s\n", entryName);
            write(snapshotFile, buffer, len);
            // Add metadata for new file
        }
    }

    // Close directory and file
    closedir(dir);
    close(snapshotFile);
}

// Helper function to find an entry in the snapshot file
int findEntryInSnapshot(int snapshotFile, const char* entryName) {
    lseek(snapshotFile, 0, SEEK_SET); // Rewind to the beginning of the file

    char line[256];
    ssize_t bytesRead;
    while ((bytesRead = read(snapshotFile, line, sizeof(line))) > 0) {
        line[bytesRead] = '\0';
        char* token = strtok(line, "\n");
        while (token != NULL) {
            if (strncmp(token, "Entry: ", 7) == 0 && strcmp(token + 7, entryName) == 0)
                return 1; // Entry found
            token = strtok(NULL, "\n");
        }
    }

    return 0; // Entry not found
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > MAX_DIRECTORIES + 2) {
        printf("Usage: %s -o <output_directory> <directory1> <directory2> ... <directoryN>\n", argv[0]);
        return 1;
    }

    char* outputDir = NULL;
    int dirStartIndex = 2;

    // Parse command-line arguments
    if (strcmp(argv[1], "-o") == 0) {
        outputDir = argv[2];
        dirStartIndex = 3;
    } else {
        printf("Usage: %s -o <output_directory> <directory1> <directory2> ... <directoryN>\n", argv[0]);
        return 1;
    }

    // Check if output directory exists, create if not
    struct stat st;
    if (stat(outputDir, &st) == -1) {
        printf("Output directory does not exist, creating...\n");
        if (mkdir(outputDir, 0777) == -1) {
            perror("Unable to create output directory");
            return 1;
        }
    }

    // Process each directory
    for (int i = dirStartIndex; i < argc; ++i) {
        captureSnapshot(argv[i], outputDir);
        monitorChanges(argv[i], outputDir);
    }

    return 0;
}
