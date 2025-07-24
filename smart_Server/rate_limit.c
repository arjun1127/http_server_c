#include "rate_limit.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 1024
#define TOKEN_REFILL_RATE 1       // 1 token per second
#define BURST_CAPACITY 5          // max 5 requests at once

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int tokens;
    time_t last_refill;
} rate_entry_t;

static rate_entry_t clients[MAX_CLIENTS];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void init_rate_limiter() {
    memset(clients, 0, sizeof(clients));
}

static rate_entry_t *get_client_entry(const char *ip) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (strcmp(clients[i].ip, ip) == 0) {
            return &clients[i];
        } else if (clients[i].ip[0] == '\0') {
            strncpy(clients[i].ip, ip, INET_ADDRSTRLEN);
            clients[i].tokens = BURST_CAPACITY;
            clients[i].last_refill = time(NULL);
            return &clients[i];
        }
    }
    return NULL; // No room
}

int is_allowed(const char *ip) {
    pthread_mutex_lock(&lock);

    rate_entry_t *entry = get_client_entry(ip);
    if (!entry) {
        pthread_mutex_unlock(&lock);
        return 0;
    }

    time_t now = time(NULL);
    int elapsed = now - entry->last_refill;
    if (elapsed > 0) {
        int refill = elapsed * TOKEN_REFILL_RATE;
        entry->tokens = (entry->tokens + refill > BURST_CAPACITY) ? BURST_CAPACITY : entry->tokens + refill;
        entry->last_refill = now;
    }

    if (entry->tokens > 0) {
        entry->tokens--;
        pthread_mutex_unlock(&lock);
        return 1;
    } else {
        pthread_mutex_unlock(&lock);
        return 0;
    }
}
