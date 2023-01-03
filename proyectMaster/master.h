#ifndef _MASTER_H_
#define _MASTER_H_

#define ON_OFF_BUTTON_PIN 13
#define POTENTIOMETER_PIN A2
#define SLAVE_ACTIVE 1
#define SLAVE_INACTIVE 0
#define ACK_RECEIVED 1
#define ACK_NOT_RECEIVED 0

int slavesAvailable[4];

void checkOnButton();
void sendMessage(char opNumber, char destination);
void handlePotentiometer();
void setupMaster();
void detectSlaves();
void turnSlavesOff();

#endif