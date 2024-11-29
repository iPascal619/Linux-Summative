#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 4
#define MAX_USERNAME 50
#define MAX_MESSAGE 1024

typedef struct {
    int socket;
    char username[MAX_USERNAME];
    int authenticated;
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_online_clients(int sender_socket) {
    char online_list[1024] = "Online Clients: ";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].authenticated && clients[i].socket != sender_socket) {
            strcat(online_list, clients[i].username);
            strcat(online_list, " ");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    send(sender_socket, online_list, strlen(online_list), 0);
}

void handle_client(void* arg) {
    int client_socket = *(int*)arg;
    char username[MAX_USERNAME], message[MAX_MESSAGE], buffer[MAX_MESSAGE];
    int authenticated = 0, client_index = -1;

    // Authentication
    send(client_socket, "Enter username: ", 16, 0);
    recv(client_socket, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = 0;

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].authenticated) {
            clients[i].socket = client_socket;
            strcpy(clients[i].username, username);
            clients[i].authenticated = 1;
            client_index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // Send online clients
    broadcast_online_clients(client_socket);

    while(1) {
        char recipient[MAX_USERNAME];
        send(client_socket, "Choose recipient: ", 18, 0);
        recv(client_socket, recipient, sizeof(recipient), 0);
        recipient[strcspn(recipient, "\n")] = 0;

        send(client_socket, "Enter message: ", 15, 0);
        recv(client_socket, message, sizeof(message), 0);
        message[strcspn(message, "\n")] = 0;

        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].authenticated && 
                strcmp(clients[i].username, recipient) == 0) {
                sprintf(buffer, "%s: %s", username, message);
                send(clients[i].socket, buffer, strlen(buffer), 0);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    pthread_t tid;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    while(1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, 
                               (struct sockaddr*)&client_addr, 
                               &addr_size);
        
        pthread_create(&tid, NULL, (void*)handle_client, &client_socket);
    }

    return 0;
}