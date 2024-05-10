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
    char path[512]; // Full path of file or directory
    time_t last_modified; // Last modified timestamp
    mode_t permissions; // File permissions
    // Add other metadata fields as needed
} Metadata;

// Function prototypes
void captureSnapshot(const char* directory, const char* inputDir, const char* outputDir);
void monitorChanges(const char* directory, const char* inputDir, const char* outputDir);
int findEntryInSnapshot(int snapshotFile, const char* entryPath);
void formatPermissions(mode_t mode, char* perm);

// Function to capture initial snapshot
void captureSnapshot(const char* directory, const char* inputDir, const char* outputDir) {
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
        snprintf(metadata.path, sizeof(metadata.path), "%s/%s", directory, entry->d_name);
        // Get last modified time and permissions
        struct stat st;
        if (stat(metadata.path, &st) == -1) {
            perror("Unable to get file stats");
            close(snapshotFile);
            closedir(dir);
            exit(EXIT_FAILURE);
        }
        metadata.last_modified = st.st_mtime;
        metadata.permissions = st.st_mode;

        // Write metadata to Snapshot.txt
        char perm[11];
        formatPermissions(metadata.permissions, perm);
        char buffer[1024];
        int len = snprintf(buffer, sizeof(buffer), "Path: %s\n", metadata.path + strlen(inputDir) + 1);
        write(snapshotFile, buffer, len);
        len = snprintf(buffer, sizeof(buffer), "Last Modified: %s", ctime(&metadata.last_modified));
        write(snapshotFile, buffer, len);
        len = snprintf(buffer, sizeof(buffer), "Permissions: %s\n", perm);
        write(snapshotFile, buffer, len);
        write(snapshotFile, "\n", 1);

        // If entry is a directory, recursively capture snapshot
        if (S_ISDIR(st.st_mode)) {
            captureSnapshot(metadata.path, inputDir, outputDir);
        }
    }

    // Close directory and file
    closedir(dir);
    close(snapshotFile);
}

// Function to monitor changes
void monitorChanges(const char* directory, const char* inputDir, const char* outputDir) {
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

        // Get entry's full path
        char entryPath[512];
        snprintf(entryPath, sizeof(entryPath), "%s/%s", directory, entry->d_name);

        // Check if entry exists in Snapshot.txt
        // If not, it's a new file
        if (!findEntryInSnapshot(snapshotFile, entryPath)) {
            printf("New file: %s\n", entryPath + strlen(inputDir) + 1);
            // Update Snapshot.txt
            char perm[11];
            struct stat st;
            if (stat(entryPath, &st) == -1) {
                perror("Unable to get file stats");
                close(snapshotFile);
                closedir(dir);
                exit(EXIT_FAILURE);
            }
            formatPermissions(st.st_mode, perm);
            char buffer[1024];
            int len = snprintf(buffer, sizeof(buffer), "Path: %s\n", entryPath + strlen(inputDir) + 1);
            write(snapshotFile, buffer, len);
            len = snprintf(buffer, sizeof(buffer), "Last Modified: %s", ctime(&st.st_mtime));
            write(snapshotFile, buffer, len);
            len = snprintf(buffer, sizeof(buffer), "Permissions: %s\n", perm);
            write(snapshotFile, buffer, len);
            write(snapshotFile, "\n", 1);
        }
    }

    // Close directory and file
    closedir(dir);
    close(snapshotFile);
}

// Helper function to find an entry in the snapshot file
int findEntryInSnapshot(int snapshotFile, const char* entryPath) {
    lseek(snapshotFile, 0, SEEK_SET); // Rewind to the beginning of the file

    char line[512];
    ssize_t bytesRead;
    while ((bytesRead = read(snapshotFile, line, sizeof(line))) > 0) {
        line[bytesRead] = '\0';
        char* token = strtok(line, "\n");
        while (token != NULL) {
            if (strncmp(token, "Path: ", 6) == 0 && strcmp(token + 6, entryPath) == 0)
                return 1; // Entry found
            token = strtok(NULL, "\n");
        }
    }

    return 0; // Entry not found
}

// Function to format permissions
void formatPermissions(mode_t mode, char* perm) {
    // Owner permissions
    perm[0] = (mode & S_IRUSR) ? 'r' : '-';
    perm[1] = (mode & S_IWUSR) ? 'w' : '-';
    perm[2] = (mode & S_IXUSR) ? 'x' : '-';
    // Group permissions
    perm[3] = (mode & S_IRGRP) ? 'r' : '-';
    perm[4] = (mode & S_IWGRP) ? 'w' : '-';
    perm[5] = (mode & S_IXGRP) ? 'x' : '-';
    // Others permissions
    perm[6] = (mode & S_IROTH) ? 'r' : '-';
    perm[7] = (mode & S_IWOTH) ? 'w' : '-';
    perm[8] = (mode & S_IXOTH) ? 'x' : '-';
    perm[9] = '\0';
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
        captureSnapshot(argv[i], argv[i], outputDir);
        monitorChanges(argv[i], argv[i], outputDir);
    }

    return 0;
}
