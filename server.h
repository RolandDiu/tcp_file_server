#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define NAME_LEN 50
#define PATH_LEN 2048
#define FOLDER "server_db"

extern int clientCount;
extern int clientSockets[MAX_CLIENTS];
extern char clientNames[MAX_CLIENTS][NAME_LEN];
extern pthread_mutex_t clientsMutex;
extern int port;

void initialize_server(int port);
void accept_connections(int serverSocket);
void *handleClient(void *socketDesc);
void sendToAll(char *msg, int len);
int nameExists(char *name);

bool parseCommand(char* buff, int client_socket, char* folder);
void showFiles(int client_socket);
void getFile(int client_socket, char* filename, char* p_path);
void putFile(int client_socket, char* filename, char* folder);
char* createFolder(char* name);

#endif
