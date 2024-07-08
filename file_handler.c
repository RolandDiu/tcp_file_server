#include "server.h"

#include <sys/stat.h>
#include <sys/types.h>

void createDirectories(char *path) {
    char *slash = strchr(path, '/');
    while (slash != NULL) {
        *slash = '\0';
        mkdir(path, 0777);
        *slash = '/';
        slash = strchr(slash + 1, '/');
    }
}

void showFiles(int client_socket) {
    char fileNames[BUFFER_SIZE] = {0};

    DIR *dir = opendir(FOLDER);

    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }
    
    struct dirent *entry;
    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Check if entry is a directory (excluding "." and "..")
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construct subdirectory path
            char subdir_path[PATH_LEN];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", FOLDER, entry->d_name);

            // Append user directory name to fileNames buffer
            strncat(fileNames, entry->d_name, NAME_LEN);
            strcat(fileNames, ":\n");

            // Open subdirectory
            DIR *subdir = opendir(subdir_path);
            if (subdir == NULL) {
                perror("Unable to open subdirectory");
                continue;
            }

            // Read files within subdirectory
            struct dirent *sub_entry;
            while ((sub_entry = readdir(subdir)) != NULL) {
                // Check if entry is a regular file
                if (sub_entry->d_type == DT_REG) {
                    // Append filename to fileNames buffer with indentation
                    strcat(fileNames, "    ");
                    strcat(fileNames, sub_entry->d_name);
                    strcat(fileNames, "\n");
                }
            }

            closedir(subdir);
        }
    }

    perror(fileNames);


    if(send(client_socket, fileNames, strlen(fileNames), 0) < 0) {
        perror("Error");
    }

    closedir(dir);
}

void putFile(int client_socket, char* filename, char* folder) {
    char path[PATH_LEN];
    sprintf(path, "%s/%s", folder, filename);
    perror(path);

    FILE *local_file = fopen(filename, "rb");
    if (local_file == NULL) {
        perror("Error opening local file for reading");
        fprintf(stderr, "Local file path: %s\n", filename);
        return;
    }

    FILE *server_file = fopen(path, "wb");
    if (server_file == NULL) {
        perror("Error opening server file for writing");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), local_file)) > 0) {
        if(fwrite(buffer, 1, bytesRead, server_file) != bytesRead) {
            perror("Error writing to server file");
            fclose(local_file);
            fclose(server_file);
            return;
        }
    }

    if (ferror(local_file)) {
        perror("Error reading from local file");
    }

    fclose(local_file);
    fclose(server_file);

}

void getFile(int client_socket, char* filename, char* p_path) {
    char path[PATH_LEN];
    sprintf(path, "%s/%s", FOLDER, p_path);

    FILE *server_file = fopen(path, "rb");
    if (server_file == NULL) {
        perror("Error opening server file for reading");
        return;
    }
    
    // Create necessary directories if they do not exist
    createDirectories(filename);

    FILE *local_file = fopen(filename, "wb");
    if (local_file == NULL) {
        perror("Error opening local file for writing");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), server_file)) > 0) {
        if(fwrite(buffer, 1, bytesRead, local_file) != bytesRead) {
            perror("Error writing to local file");
            fclose(local_file);
            fclose(server_file);
            return;
        }
    }

    if (ferror(server_file)) {
        perror("Error reading from server file");
    }

    fclose(local_file);
    fclose(server_file);
}