#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define LISTENADDR "127.0.0.1"
char *error;

int srv_init(int portno) {
    int s;
    struct sockaddr_in srv;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        error = "socket() error";
        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);
    srv.sin_port = htons(portno);

    if (bind(s, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        close(s);
        error = "bind() error";
        return 0;
    }

    if (listen(s, 5) < 0) {
        close(s);
        error = "listen() error";
        return 0;
    }

    return s;
}

int cli_accept(int s) {
    int c;
    socklen_t addrlen;
    struct sockaddr_in cli;

    addrlen = sizeof(cli);
    memset(&cli, 0, sizeof(cli));
    c = accept(s, (struct sockaddr *)&cli, &addrlen);
    if (c < 0) {
        error = "accept() error";
        return 0;
    }
    return c;
}

void cli_conn(int s, int c) {
    return;  // Will implement later
}

int main(int argc, char *argv[]) {
    int s, c;
    char *port;

    // To prevent zombie processes
    signal(SIGCHLD, SIG_IGN);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <listening port>\n", argv[0]);
        return -1;
    } else {
        port = argv[1];
    }

    s = srv_init(atoi(port));
    if (!s) {
        fprintf(stderr, "%s\n", error ? error : "Unknown error");
        return -1;
    }

    printf("Listening on %s:%s\n", LISTENADDR, port);

    while (1) {
        c = cli_accept(s);
        if (c <= 0) {
            fprintf(stderr, "%s\n", error ? error : "Unknown error");
            continue;
        }

        printf("Incoming connection\n");

        if (!fork()) {
            cli_conn(s, c);
            close(c);  // optional: close client socket in child
            exit(0);
        }

        close(c);  // parent closes client socket
    }

    return 0;
}
