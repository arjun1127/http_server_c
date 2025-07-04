#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "rate_limit.h"

#define PORT 8282
#define BUFFER_SIZE 8192

void handle_client(int client_sock, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE] = {0};
    recv(client_sock, buffer, sizeof(buffer) - 1, 0);

    char *client_ip = inet_ntoa(client_addr.sin_addr);
    printf("Client connected: %s\n", client_ip);

    if (!is_allowed(client_ip)) {
        const char *response =
            "HTTP/1.1 429 Too Many Requests\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Rate limit exceeded. Try again later.\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
        exit(0);
    }

    char method[8];
    sscanf(buffer, "%s", method);

    if (strcmp(method, "POST") == 0) {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body) body += 4;

        char name[256] = "", email[256] = "";
        sscanf(body, "name=%255[^&]&email=%255[^\r\n]", name, email);

        char response[1024];
        snprintf(response, sizeof(response),
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><body><h1>Hello, %s!</h1><p>Email: %s</p></body></html>\n",
            name, email);
        send(client_sock, response, strlen(response), 0);
    } else {
        const char *response =
            "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
            "<html><body><h1>Welcome to My C HTTP Server</h1></body></html>\n";
        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    signal(SIGCHLD, SIG_IGN);
    init_rate_limiter();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on http://127.0.0.1:%d\n", PORT);

    while (1) {
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_client(client_sock, address);
        } else {
            close(client_sock);
        }
    }
    return 0;
}