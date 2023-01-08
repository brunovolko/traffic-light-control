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

void messageReceivedHandler() {
  int sender, opNumber, destination, integrity;
  while(4 <= Wire.available()) {
    // Each command has 4 bytes
    sender = Wire.read();
    opNumber = Wire.read();
    destination = Wire.read();
    integrity = Wire.read();
    if(integrity = (sender + opNumber + destination)) {
      Serial.println("Integrity passed");      
    } else {
      Serial.println("Integrity mismatch when receiving a message.");
    }

  }
  /*while (3 <= Wire.available()) {
    
    
    if (prot <= 2 && prot >= 0){
      int x = Wire.read(); // read the next byte as an int
      int y = Wire.read(); // read the next byte as an int
      int number = x | y << 8;
      switch(prot) { //depending on the protocol, the function to handle the LEDs
        case TEMP:
          manageTempLight(number);
          break;
        case LIGHT:
          manageLightLight(number);
          break;
        case ROTATION:
          //manageRotLight(number);
          break;
        default:
          return;
      }
    }
  }*/
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
