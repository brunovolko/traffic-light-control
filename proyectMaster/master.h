#ifndef _MASTER_H_
#define _MASTER_H_

#define ON_OFF_BUTTON_PIN 13
#define STATUS_LED_PIN 12
#define POTENTIOMETER_PIN A2
#define SLAVE_ACTIVE 1
#define SLAVE_INACTIVE 0
#define ACK_RECEIVED 1
#define ACK_NOT_RECEIVED 0
#define TIME_DELAY_TO_CHECK_ADDRESS 1000

int slavesAvailable[4];
int currentEntry = 0; //Stores the current entry with green light to enter
long lastTrafficLightUpdate = 0; //

void checkOnButton();
void sendMessage(char opNumber, char destination);
void handlePotentiometer();
void setupMaster();
void detectSlaves();
void orchestrate();
void turnSlavesOff();

#endif