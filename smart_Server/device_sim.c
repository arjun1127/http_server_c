// smart_server/device_sim.c
#include "device_sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int led_state = 0;

float get_temperature() {
    srand(time(NULL) ^ rand());
    return 25.0f + (rand() % 1000) / 100.0f; // Simulate 25.00 - 34.99 C
}

int set_led(int state) {
    led_state = state ? 1 : 0;
    return led_state;
}

int get_led() {
    return led_state;
}

void get_device_status(char *buffer, size_t bufsize) {
    snprintf(buffer, bufsize,
        "{\"temperature\": %.2f, \"led\": \"%s\"}",
        get_temperature(),
        get_led() ? "on" : "off");
}
