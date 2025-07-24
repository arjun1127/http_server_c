#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// Decode percent-encoded URL values (e.g., %20 -> space)
void url_decode(char *src) {
    char *dest = src;
    while (*src) {
        if (*src == '%') {
            if (src[1] && src[2]) {
                char hex[3] = {src[1], src[2], '\0'};
                *dest++ = (char) strtol(hex, NULL, 16);
                src += 3;
            }
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

// Trim leading and trailing whitespace (in-place)
void trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';
}
