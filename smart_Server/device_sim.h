#ifndef DEVICE_SIM_H
#define DEVICE_SIM_H

#include <stddef.h>

// Simulate temperature sensor (returns a float)
float get_temperature();

// Simulate LED actuator: sets on (1) or off (0), returns new state
int set_led(int state);

// Returns the current LED state
int get_led();

// Writes JSON status of device into given buffer
void get_device_status(char *buffer, size_t bufsize);

#endif
