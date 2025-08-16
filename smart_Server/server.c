#include "server.h"
#include "rate_limit.h"
#include "device_sim.h"
#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// close(int fd) → used at the end to close the socket connection.
// read() / write() are also in this header (though we are using recv() and send() from <sys/socket.h>).
#include <unistd.h>
// Functions for Internet address manipulation (IPv4/IPv6).
#include <arpa/inet.h>

//wkt that the response type for a server should include the status, content-type, length,and connection so define a function for that 
//as output we can see the response string 
void send_response(int client_sock, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n\r\n"
        "%s",
        status, content_type, strlen(body), body);
        //send() transmits the data over the TCP socket to the browser/client.
    send(client_sock, response, strlen(response), 0);
}
//example res should be : Example for status="200 OK", content_type="application/json", body="{\"temperature\":28.5}":


// Extracts the value of a query parameter from a URL.
// Example:
// /led?state=on → key="state" → returns "on".

//follow this algorithm to pasrse the query

// strstr(query, key) → finds "state" in "state=on".
// Moves pointer after "state".
// Checks = is there.
// Finds end (either & or \0).
// Copies value into value[].
// url_decode(value) → decodes things like %20 → space.
// Returns the parameter’s value.
// So this is the parser for query strings.


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


//parsing http request
// Splits the HTTP request into:
// HTTP method (e.g., "GET", "POST").
// Path (e.g., "/led").
// Query string (e.g., "state=on").
// How?
// sscanf(request, "%s %s", method, path);
// From "GET /led?state=on HTTP/1.1", it extracts:
// method="GET"
// path="/led?state=on".
// strchr(path, '?') → finds ?.
// Cuts path at ?, copies "state=on" into query.
// This enables the routing logic later.
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
//     Allocates a receive buffer.
// Extracts client_sock (the TCP connection fd).
// Gets client’s IP address (inet_ntoa).
    char buffer[BUFFER_SIZE] = {0};
    int client_sock = client->client_sock;
    struct sockaddr_in addr = client->client_addr;
    char *client_ip = inet_ntoa(addr.sin_addr);

    // Rate limiting
    if (!is_allowed(client_ip)) {
// Uses rate_limit.c to check if this IP has exceeded request limits.
// If yes → sends HTTP 429 Too Many Requests and kills connection.
// Prevents DoS/spam.
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

// We need all this because HTTP is not just “sending numbers.” It’s a protocol with rules (request → parse → process → structured response). Each part ensures:
// Networking layer works (headers).
// Requests are understood (parse_http_request).
// Actions are meaningful (get_query_param, routing).
// Responses are valid (send_response).
// Server stays stable (handle_client, cleanup, rate limiting).
