#include "server.h"

void *handleClient(void *socketDesc) {
    int sock = *(int *)socketDesc;
    free(socketDesc);

    char name[NAME_LEN] = {0};
    char recvBuf[BUFFER_SIZE];
    char sendBuf[BUFFER_SIZE + NAME_LEN];
    bool isNameAccepted = false;

    const char *usernamePrompt = "Input the username\n"; 
    send(sock, usernamePrompt, strlen(usernamePrompt), 0);

    char folder[PATH_LEN] = "";

    // Request a name from the client until a unique one is provided
    while (!isNameAccepted) {
        memset(name, 0, NAME_LEN);
        int bytesReceived = recv(sock, name, NAME_LEN - 1, 0);
        
        if (bytesReceived <= 0) {
            perror("Failed to get name from client");
            close(sock);
            return NULL;
        }

        name[bytesReceived] = '\0';
        name[strcspn(name, "\r\n")] = 0;

        pthread_mutex_lock(&clientsMutex);

        if (!nameExists(name)) {
            strncpy(clientNames[clientCount], name, NAME_LEN - 1);
            clientSockets[clientCount++] = sock;
            isNameAccepted = true;
            const char *nameAccepted = "Username accepted\n";
            send(sock, nameAccepted, strlen(nameAccepted), 0);
            printf("port %d\n", port);

            // Create a folder for this username
            strcpy(folder, createFolder(name));
        } else {
            const char *nameRejected = "Username rejected\n";
            send(sock, nameRejected, strlen(nameRejected), 0);
            pthread_mutex_unlock(&clientsMutex);
            close(sock);
            return NULL;
        }
        pthread_mutex_unlock(&clientsMutex);
    }    

    // Handle regular chat messages
    while (1) {
        memset(recvBuf, 0, BUFFER_SIZE);
        int receive = recv(sock, recvBuf, BUFFER_SIZE, 0);
        if (receive > 0) {
            // Parse if get/put command is used
            if(!parseCommand(recvBuf, sock, folder)) {
                snprintf(sendBuf, sizeof(sendBuf), "Message:%s: %s", name, recvBuf);
                sendToAll(sendBuf, strlen(sendBuf));
            }
        } else 
            break;
    }

    // Cleanup
    pthread_mutex_lock(&clientsMutex);
    int i;
    for (i = 0; i < clientCount; i++) {
        if (clientSockets[i] == sock) {
            for (int j = i; j < clientCount - 1; j++) {
                clientSockets[j] = clientSockets[j + 1];
                strncpy(clientNames[j], clientNames[j + 1], NAME_LEN);
            }
            clientCount--;
            break;
        }
    }
    pthread_mutex_unlock(&clientsMutex);

    close(sock);
    return NULL;
}

void sendToAll(char *msg, int len) {
    pthread_mutex_lock(&clientsMutex);
    for (int i = 0; i < clientCount; i++) {
        if (send(clientSockets[i], msg, len, 0) < 0) {
            perror("Error sending message");
            continue;
        }
    }
    pthread_mutex_unlock(&clientsMutex);
}

int nameExists(char *name) {
    for (int i = 0; i < clientCount; i++) {
        if (strcmp(clientNames[i], name) == 0) return 1;
    }
    return 0;
}

// Create folder "S" + port + ":" + username
char* createFolder(char* name) {
    char fullName[PATH_LEN];

    sprintf(fullName, "%s/S%d:%s", FOLDER, port, name);

    // Check if the directory already exists
    struct stat st;
    if (stat(fullName, &st) == 0) {
        return strdup(fullName);
    }

    if (mkdir(fullName, 0777) == -1)
    {
        perror("Error");
        return NULL;
    }

    return strdup(fullName);
}

bool parseCommand(char* buff, int client_socket, char* folder) {
    if(strncmp(buff, "@show", 4) == 0) {
        showFiles(client_socket);
        return true;
    }
    else if (strncmp(buff, "@put", 4) == 0) {
        char* filename = strtok(buff + 4, " "); // Parse the filename
        // Trim whitespace characters from the filename
        char *trimFilename = strtok(filename, "\r\n\t ");
        if (trimFilename != NULL) {
            
            perror(trimFilename); // Feedback in the server

            putFile(client_socket, trimFilename, folder);
            
            return true;
        }
    }
    else if (strncmp(buff, "@get", 4) == 0) {
        char* path_and_filename = strtok(buff + 4, " ");
        char* path = strtok(path_and_filename, "\r\n\t ");
        
        char* slash_position = strchr(path_and_filename, '/'); // Find the position of the "/"

        if (slash_position != NULL) {
            char* filename = slash_position + 1; // Move to the filename
            perror(filename);
            perror(path);
            perror(path_and_filename);

            getFile(client_socket, filename, path);

            return true;
        }
    }
        
    return false;
}