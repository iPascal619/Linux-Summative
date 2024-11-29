#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <pthread.h>

#define MAX_DEVICES 10
#define LOG_FILE "network_devices.txt"
#define SERVER_PORT 8888

typedef struct {
    char ip_address[INET_ADDRSTRLEN];
    char hostname[NI_MAXHOST];
} NetworkDevice;

// Global variables for server-client communication
NetworkDevice discovered_devices[MAX_DEVICES];
int device_count = 0;
pthread_mutex_t device_mutex = PTHREAD_MUTEX_INITIALIZER;

void discover_network_devices() {
    struct ifaddrs *ifap, *ifa;
    device_count = 0;

    if (getifaddrs(&ifap) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ipstr, sizeof(ipstr));

            // Skip loopback and invalid IPs
            if (strcmp(ipstr, "127.0.0.1") == 0 || 
                strncmp(ipstr, "169.254", 7) == 0) continue;

            char hostname[NI_MAXHOST] = {0};
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), 
                        hostname, NI_MAXHOST, 
                        NULL, 0, NI_NAMEREQD);

            pthread_mutex_lock(&device_mutex);
            if (device_count < MAX_DEVICES) {
                strcpy(discovered_devices[device_count].ip_address, ipstr);
                strcpy(discovered_devices[device_count].hostname, 
                       strlen(hostname) > 0 ? hostname : "Unknown");
                device_count++;
            }
            pthread_mutex_unlock(&device_mutex);
        }
    }

    freeifaddrs(ifap);
}

// Server function to log network devices
void* server_thread(void* arg) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    FILE *log_file;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return NULL;
    }

    // Listen for connections
    listen(server_socket, 5);
    printf("Server listening on port %d\n", SERVER_PORT);

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, 
                               (struct sockaddr*)&client_addr, 
                               &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Open log file
        log_file = fopen(LOG_FILE, "a");
        if (!log_file) {
            perror("Could not open log file");
            close(client_socket);
            continue;
        }

        // Log devices received from client
        pthread_mutex_lock(&device_mutex);
        for (int i = 0; i < device_count; i++) {
            char log_entry[256];
            snprintf(log_entry, sizeof(log_entry), 
                     "IP: %s, Hostname: %s\n", 
                     discovered_devices[i].ip_address, 
                     discovered_devices[i].hostname);
            
            // Write to log file
            fputs(log_entry, log_file);
            
            // Send to client
            send(client_socket, log_entry, strlen(log_entry), 0);
        }
        pthread_mutex_unlock(&device_mutex);

        // Close file and client connection
        fclose(log_file);
        close(client_socket);
    }

    close(server_socket);
    return NULL;
}

// Client function to send network devices to server
void* client_thread(void* arg) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Client socket creation failed");
        return NULL;
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return NULL;
    }

    // Discover network devices
    discover_network_devices();

    // Prepare device information to send
    for (int i = 0; i < device_count; i++) {
        snprintf(buffer, sizeof(buffer), 
                 "Device %d - IP: %s, Hostname: %s\n", 
                 i+1, 
                 discovered_devices[i].ip_address, 
                 discovered_devices[i].hostname);
        
        // Send device info to server
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
    return NULL;
}

int main() {
    pthread_t server_tid, client_tid;

    // Create server thread
    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("Server thread creation failed");
        return 1;
    }

    // Small delay to ensure server is up
    sleep(1);

    // Create client thread
    if (pthread_create(&client_tid, NULL, client_thread, NULL) != 0) {
        perror("Client thread creation failed");
        return 1;
    }

    // Wait for threads to complete
    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);

    return 0;
}