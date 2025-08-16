//  smart_server/device_sim.c
//this below code is just to show a dummy sensor 
// #include "device_sim.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>

// static int led_state = 0;

// float get_temperature() {
//     srand(time(NULL) ^ rand());
//     return 25.0f + (rand() % 1000) / 100.0f; // Simulate 25.00 - 34.99 C
// }

// int set_led(int state) {
//     led_state = state ? 1 : 0;
//     return led_state;
// }

// int get_led() {
//     return led_state;
// }

// void get_device_status(char *buffer, size_t bufsize) {
//     snprintf(buffer, bufsize,
//         "{\"temperature\": %.2f, \"led\": \"%s\"}",
//         get_temperature(),
//         get_led() ? "on" : "off");
// }

//if we were to measure with an actual sensor than we can use DHT sensor with ESP32 or something along the way 
//it doesnt matter how complex your hardware , we can show the results using this program 

#include "device_real.h"
#include <stdio.h>
#include "DHT.h"

// Define sensor type and GPIO pin
#define DHTPIN 4        // GPIO4 (example pin)
#define DHTTYPE DHT22   // or DHT11

static DHT dht(DHTPIN, DHTTYPE);
static int led_state = 0;

// Initialize the device (sensor + LED GPIO)
void device_init() {
    dht.begin();
    pinMode(2, OUTPUT);   // Example: LED on GPIO2
}

float get_temperature() {
    float t = dht.readTemperature();
    if (isnan(t)) {
        return -999; // error code
    }
    return t;
}

int set_led(int state) {
    led_state = state ? 1 : 0;
    digitalWrite(2, led_state); // Control real LED
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

//now how communication happens ? 
//’s the chain now:

// Client request arrives at server
// Example: GET /status
// Server calls get_device_status(buffer, size)
// This function calls get_temperature() and get_led().
// get_temperature() queries the real sensor hardware via GPIO.
// ESP32 pulls data bits from the DHT sensor.
// set_led() toggles a real LED connected to GPIO.
// No longer just a variable — an actual pin changes voltage.
// Server sends JSON back to client
// Example:
// {"temperature": 28.50, "led": "on"}


