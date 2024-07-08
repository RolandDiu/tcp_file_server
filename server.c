#include "server.h"

int clientCount = 0;
int clientSockets[MAX_CLIENTS];
char clientNames[MAX_CLIENTS][NAME_LEN];
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;

void initialize_server(int port) {
    int serverSocket;
    struct sockaddr_in6 serverAddr;

    serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        exit(2);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_addr = in6addr_any;
    serverAddr.sin6_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        exit(3);
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        close(serverSocket);
        exit(4);
    }

    printf("Server listening on port %d\n", port);
    accept_connections(serverSocket);
}

void accept_connections(int serverSocket) {
    struct sockaddr_in6 clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (1) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_mutex_lock(&clientsMutex);

        if (clientCount < MAX_CLIENTS) {
            int* newSockPtr = malloc(sizeof(int));
            *newSockPtr = clientSocket;

            pthread_t threadID;
            pthread_create(&threadID, NULL, handleClient, (void *)newSockPtr);
        } else {
            printf("Max clients reached. Connection refused: ");
            close(clientSocket);
        }

        pthread_mutex_unlock(&clientsMutex);
    }
}