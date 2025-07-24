#ifndef RATE_LIMIT_H
#define RATE_LIMIT_H

#include <netinet/in.h>

void init_rate_limiter();
int is_allowed(const char *ip);

#endif
