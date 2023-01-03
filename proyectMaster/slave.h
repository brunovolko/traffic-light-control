#ifndef _SLAVE_H_
#define _SLAVE_H_

#include "constants.h"

#define INTERIOR_RED_PIN 11
#define INTERIOR_YELLOW_PIN 10
#define INTERIOR_GREEN_PIN 9

#define INCOMING_RED_PIN 8
#define INCOMING_YELLOW_PIN 7
#define INCOMING_GREEN_PIN 6

#define PEDESTRIAN_GREEN_PIN 5

void setupSlave();
void turnLightsOff();
void blink();

char blinking = BLINKING_ON;
char status_light_incoming = NO_LIGHTS;
char status_light_inside = NO_LIGHTS;
char status_light_pedestrian = NO_LIGHTS;

#endif