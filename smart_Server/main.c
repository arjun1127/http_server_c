#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "server.h"
#include "rate_limit.h"

#define PORT 8282

// Thread function for each client
void *client_thread(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    handle_client(info);
    free(info);  // Free memory after done
    return NULL;
}

// Create and configure the server socket
int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int main() {
    int server_fd;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    signal(SIGPIPE, SIG_IGN);  // Prevent crash on broken pipe
    init_rate_limiter();       // Initialize rate limiter

    server_fd = create_server_socket(PORT);
    if (server_fd < 0) {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("[+] Smart Embedded C HTTP Server started at http://127.0.0.1:%d\n", PORT);

    while (1) {
        int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_sock < 0) {
            perror("[-] Accept failed");
            continue;
        }

        // Allocate and populate client_info struct
        client_info_t *info = malloc(sizeof(client_info_t));
        if (!info) {
            perror("[-] Malloc failed");
            close(client_sock);
            continue;
        }

        info->client_sock = client_sock;
        info->client_addr = client_addr;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, (void *)info) != 0) {
            perror("[-] Thread creation failed");
            close(client_sock);
            free(info);
            continue;
        }

        pthread_detach(tid);  // Automatically clean up after thread finishes
    }

    close(server_fd);
    return 0;
}
