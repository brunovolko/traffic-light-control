#include "slave.h"
#include "constants.h"

long lastBlinkChange = 0, currentTime;

void setupSlave() {
  pinMode(INTERIOR_RED_PIN, OUTPUT);
  pinMode(INTERIOR_YELLOW_PIN, OUTPUT);
  pinMode(INTERIOR_GREEN_PIN, OUTPUT);
  pinMode(INCOMING_RED_PIN, OUTPUT);
  pinMode(INCOMING_YELLOW_PIN, OUTPUT);
  pinMode(INCOMING_GREEN_PIN, OUTPUT);
  pinMode(PEDESTRIAN_GREEN_PIN, OUTPUT);

  pinMode(ADDRESS_PIN_0, INPUT);
  pinMode(ADDRESS_PIN_1, INPUT);
}

void turnLightsOff() {
  digitalWrite(INTERIOR_RED_PIN, LOW);
  digitalWrite(INTERIOR_YELLOW_PIN, LOW);
  digitalWrite(INTERIOR_GREEN_PIN, LOW);
  digitalWrite(INCOMING_RED_PIN, LOW);
  digitalWrite(INCOMING_YELLOW_PIN, LOW);
  digitalWrite(INCOMING_GREEN_PIN, LOW);
  digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);
}

void handleOperation(int opNumber) {
  //RED_CMD, GREEN_CMD, OFF_CMD, PING_CMD, ACK, STATUS_CMD
  switch(opNumber) {
    case RED_CMD:
      //TODO
      break;
    case GREEN_CMD:
      //TODO
      break;
    case OFF_CMD:
      turnLightsOff();
      blinking = BLINKING_ON;
      break;
  }
}

void messageReceivedHandler() {
  int sender, opNumber, destination, integrity;
  while(4 <= Wire.available()) {
    // Each command has 4 bytes
    sender = Wire.read();
    opNumber = Wire.read();
    destination = Wire.read();
    integrity = Wire.read();
    if(integrity = (sender + opNumber + destination)) {
      request_received = opNumber;
      handleOperation(request_received);
    } else {
      Serial.println("Integrity mismatch when receiving a message.");
    }

  }
}

void requestReceivedHandler() {
  if(request_received == PING_CMD) {
    char response [FIVE_BYTES];
    unsigned char information = 0; //0 means no failures.
    //We cant check if a light is failing because we send digitalWrite commands but we dont get an answer
    information = (pedestrian_button_pressed ? 2 : 0); //2 equals to a binary 10. Therefore the 7th bit is affected.

    if (!sprintf(response, "%c%c%c%c%c", localAddress, STATUS_CMD, 0, information, localAddress+STATUS_CMD+information)) { //0 is orchestrator
      Serial.println("BROKEN SPRINTF");
    } else {
      Wire.write(response, FIVE_BYTES);
    }     
  } else if(request_received != NOTHING) {
    char response [FOUR_BYTES];
    if (!sprintf(response, "%c%c%c%c", localAddress, ACK, 0, ACK+localAddress)) { //0 is orchestrator
      Serial.println("BROKEN SPRINTF");
    } else {
      Wire.write(response, FOUR_BYTES);
    }
  }
  request_received = NOTHING;
}

void blink() {
  if (lastBlinkChange == 0) {
    lastBlinkChange = millis();
  } else {
    currentTime = millis();
    if(currentTime - lastBlinkChange > DELAY_YELLOW_BLINK) {
      lastBlinkChange = currentTime; //Reset time
      //Interior traffic light
      status_light_inside = (status_light_inside == NO_LIGHTS ? YELLOW_LIGHT : NO_LIGHTS);
      digitalWrite(INTERIOR_YELLOW_PIN, status_light_inside == YELLOW_LIGHT ? HIGH : LOW); //Send signal to led
      //Incoming traffic light
      status_light_incoming = (status_light_incoming == NO_LIGHTS ? YELLOW_LIGHT : NO_LIGHTS);
      digitalWrite(INCOMING_YELLOW_PIN, status_light_incoming == YELLOW_LIGHT ? HIGH : LOW);
      //Pedestrian traffic light
      status_light_pedestrian = (status_light_pedestrian == NO_LIGHTS ? GREEN_LIGHT : NO_LIGHTS);
      digitalWrite(PEDESTRIAN_GREEN_PIN, status_light_pedestrian == GREEN_LIGHT ? HIGH : LOW);
    }
    
  }
  
}
