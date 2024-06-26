#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MAX_DIRECTORIES 10

typedef struct {
    char name[256]; // Name of file or directory
    char path[512]; // Full path of file or directory
    time_t last_modified; // Last modified timestamp
    mode_t permissions; // File permissions
    // Add other metadata fields as needed
} Metadata;

// Function prototypes
void captureSnapshot(const char* maldir, const char* directory, const char* inputDir, const char* outputDir, const char* snapshotFilePath);
void monitorChanges(const char* directory, const char* inputDir, const char* outputDir);
int findEntryInSnapshot(int snapshotFile, const char* entryPath);
void formatPermissions(mode_t mode, char* perm);

// Function to capture initial snapshot
void captureSnapshot(const char* maldir, const char* directory, const char* inputDir, const char* outputDir, const char* snapshotFilePath) {

    // Open the directory
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        perror("Unable to open directory");
        exit(EXIT_FAILURE);
    }

    

    // Create directory-specific Snapshot file
    char newSnapshotFilePath[512];
    if(strcmp(snapshotFilePath, "\0")) {
        strcpy(newSnapshotFilePath, snapshotFilePath);
    } else {
        char sanitizedDirName[512];
        strcpy(sanitizedDirName, directory);
        char* p = sanitizedDirName;
        while (*p) {
        if (*p == '.' || *p == '/') {
            *p = '_'; // Replace dot and backslash with underscore
        }
        p++;
    }
        snprintf(newSnapshotFilePath, sizeof(newSnapshotFilePath) + sizeof(sanitizedDirName), "%s/Snapshot_%s.txt", outputDir, sanitizedDirName);
    }
    int snapshotFile = open(newSnapshotFilePath, O_WRONLY | O_APPEND | O_CREAT, 0777);
    if (snapshotFile == -1) {
        perror("Unable to create/open Snapshot file");
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
        printf("%s\n", metadata.name);
        printf("%s\n", metadata.path);
        // Get last modified time and permissions
            struct stat st;
            if (stat(metadata.path, &st) == -1) {
                perror("Unable to get file stats");
                close(snapshotFile);
                closedir(dir);
                exit(EXIT_FAILURE);
            }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }   


        // Child process
        else if (pid == 0 && !S_ISDIR(st.st_mode)) { 
            // Execute the bash script with the file to be snapshot and the directory
            char script[150] = "./analyze.sh";
            
                 

            execl(script, script, metadata.path, maldir, NULL);
            
            // If execl fails
            perror("execl failed");
            exit(EXIT_FAILURE);
        } else {
        

            int status;
            pid_t vpid = wait(&status);
            if (vpid != -1) {
                if (WIFEXITED(status)) {
                    printf("Child process with PID %d terminated with exit code %d\n", pid, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("Child process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
                }
            }
            
            
        
            metadata.last_modified = st.st_mtime;
            metadata.permissions = st.st_mode;

            // Write metadata to Snapshot file
            char perm[11];
            formatPermissions(metadata.permissions, perm);
            char buffer[1024];
            int len = snprintf(buffer, sizeof(metadata.path), "Path: %s\n", metadata.path);
            write(snapshotFile, buffer, len);
            len = snprintf(buffer, sizeof(buffer), "Last Modified: %s", ctime(&metadata.last_modified));
            write(snapshotFile, buffer, len);
            len = snprintf(buffer, sizeof(buffer), "Permissions: %s\n", perm);
            write(snapshotFile, buffer, len);
            write(snapshotFile, "\n", 1);
            
            // If entry is a directory, recursively capture snapshot
            if (S_ISDIR(st.st_mode)) {
                captureSnapshot(maldir, metadata.path, inputDir, outputDir, newSnapshotFilePath);
            }
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
            printf("New file: %s/%s\n", inputDir, entry->d_name);
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
            int len = snprintf(buffer, sizeof(buffer), "Path: %s/%s\n", inputDir, entry->d_name);
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
    const char malwareDir[20] = "./quarantine";
    // Parse command-line arguments
    if (strcmp(argv[1], "-o") == 0) {
        outputDir = argv[2];
        dirStartIndex = 3;
    } else {
        printf("Usage: %s -o <output_directory> <directory1> <directory2> ... <directoryN>\n", argv[0]);
        return 1;
    }

    // Check if output directory exists, create if not
    if (mkdir(outputDir, S_IRWXU) == -1) {

        if(errno != EEXIST){
            perror("Unable to create output directory");
        return 1;
        }
        
    }

    // Process each directory
    for (int i = dirStartIndex; i < argc; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            return 1;
        }   
        // Child process
        else if (pid == 0) { 
            const char aux[512] = "\0";
            captureSnapshot(malwareDir, argv[i], argv[i], outputDir, aux);
            exit(0);
        }
    }

    // Wait for all child processes to finish and print their status
    for (int i = dirStartIndex; i < argc; ++i) {
        int status;
        pid_t pid = wait(&status);
        if (pid != -1) {
            if (WIFEXITED(status)) {
                printf("Child process with PID %d terminated with exit code %d\n", pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
            }
        }
    }

    return 0;
}
