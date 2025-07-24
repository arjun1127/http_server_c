#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

#define BUFFER_SIZE 4096

typedef struct {
    int client_sock;
    struct sockaddr_in client_addr;
} client_info_t;

// Handles an individual client connection
void handle_client(client_info_t *client);

// Sends a simple HTTP response
void send_response(int client_sock, const char *status, const char *content_type, const char *body);

// Parses query parameter from URL (basic, single key-value)
char *get_query_param(const char *query, const char *key);

// Extracts the requested path from HTTP GET line
void parse_http_request(const char *request, char *method, char *path, char *query);

#endif
