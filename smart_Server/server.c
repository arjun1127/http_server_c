#include "server.h"
#include "rate_limit.h"
#include "device_sim.h"
#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void send_response(int client_sock, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n\r\n"
        "%s",
        status, content_type, strlen(body), body);
    send(client_sock, response, strlen(response), 0);
}

char *get_query_param(const char *query, const char *key) {
    static char value[256];
    char *start = strstr(query, key);
    if (!start) return NULL;

    start += strlen(key);
    if (*start != '=') return NULL;
    start++;

    char *end = strchr(start, '&');
    if (!end) end = strchr(start, '\0');

    int len = end - start;
    if (len >= sizeof(value)) len = sizeof(value) - 1;

    strncpy(value, start, len);
    value[len] = '\0';
    url_decode(value);
    return value;
}

void parse_http_request(const char *request, char *method, char *path, char *query) {
    sscanf(request, "%s %s", method, path);

    // Extract query string if exists
    char *q = strchr(path, '?');
    if (q) {
        *q = '\0';
        strcpy(query, q + 1);
    } else {
        query[0] = '\0';
    }
}

void handle_client(client_info_t *client) {
    char buffer[BUFFER_SIZE] = {0};
    int client_sock = client->client_sock;
    struct sockaddr_in addr = client->client_addr;
    char *client_ip = inet_ntoa(addr.sin_addr);

    // Rate limiting
    if (!is_allowed(client_ip)) {
        send_response(client_sock, "429 Too Many Requests", "text/plain", "Rate limit exceeded. Try again later.");
        close(client_sock);
        free(client);
        pthread_exit(NULL);
    }

    // Read request
    ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_sock);
        free(client);
        pthread_exit(NULL);
    }

    char method[8], path[256], query[512];
    parse_http_request(buffer, method, path, query);

    // Routing
    if (strcmp(path, "/sensor/temp") == 0 && strcmp(method, "GET") == 0) {
        float temp = get_temperature();
        char body[128];
        snprintf(body, sizeof(body), "{\"temperature\": %.2f}", temp);
        send_response(client_sock, "200 OK", "application/json", body);

    } else if (strcmp(path, "/led") == 0 && strcmp(method, "POST") == 0) {
        const char *state = get_query_param(query, "state");
        if (state) {
            int result = set_led(strcmp(state, "on") == 0 ? 1 : 0);
            char body[64];
            snprintf(body, sizeof(body), "{\"led\": \"%s\"}", result ? "on" : "off");
            send_response(client_sock, "200 OK", "application/json", body);
        } else {
            send_response(client_sock, "400 Bad Request", "text/plain", "Missing 'state' query parameter.");
        }

    } else if (strcmp(path, "/device/status") == 0 && strcmp(method, "GET") == 0) {
        char body[256];
        get_device_status(body, sizeof(body));
        send_response(client_sock, "200 OK", "application/json", body);

    }else if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        const char *html =
            "<!DOCTYPE html><html><head><title>Smart Device Server</title></head><body>"
            "<h1>Smart Embedded C HTTP Server</h1>"
            "<h2>Test Controls</h2>"
            "<ul>"
            "<li><a href='/sensor/temp'> Read Temperature</a></li>"
            "<li><a href='/device/status'> Device Status</a></li>"
            "</ul>"
            "<h3>Toggle LED</h3>"
            "<form action='/led' method='POST'>"
            "  <label for='state'>State:</label>"
            "  <select name='state'>"
            "    <option value='on'>ON</option>"
            "    <option value='off'>OFF</option>"
            "  </select>"
            "  <input type='submit' value='Toggle' />"
            "</form>"
            "<p style='font-size:small;'>Made in C with </p>"
            "</body></html>";
        send_response(client_sock, "200 OK", "text/html", html);
    } 
    
    else {
        send_response(client_sock, "404 Not Found", "text/plain", "Resource not found.");
    }

    close(client_sock);
    free(client);
    pthread_exit(NULL);
}
