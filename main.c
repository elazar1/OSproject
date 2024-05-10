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

}


int main(){
    if(argc != 2){
        printf("Usage: %s <directory>\n", argv[0]);
        return 1;
    }
    return 0;
}