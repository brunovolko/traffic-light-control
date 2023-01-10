#include "slave.h"
#include "constants.h"

long lastBlinkChange = 0, currentTime;
char isPressingPedestrianButton = NOT_PRESSING_BUTTON;
unsigned long PedestrianButtonlastDebounceTime = 0;

void setupSlave() {
  pinMode(INTERIOR_RED_PIN, OUTPUT);
  pinMode(INTERIOR_YELLOW_PIN, OUTPUT);
  pinMode(INTERIOR_GREEN_PIN, OUTPUT);
  pinMode(INCOMING_RED_PIN, OUTPUT);
  pinMode(INCOMING_YELLOW_PIN, OUTPUT);
  pinMode(INCOMING_GREEN_PIN, OUTPUT);
  pinMode(PEDESTRIAN_GREEN_PIN, OUTPUT);
  pinMode(PEDESTRIAN_BUTTON_PIN, INPUT);

  pinMode(ADDRESS_PIN_0, INPUT);
  pinMode(ADDRESS_PIN_1, INPUT);
}

void checkPedestrianButton() { //Verifies the pin of the pedestrian button
  int pedestrianButtonState = digitalRead(PEDESTRIAN_BUTTON_PIN);
  if(pedestrianButtonState == HIGH && !isPressingPedestrianButton) {
    isPressingPedestrianButton = PRESSING_BUTTON;
    PedestrianButtonlastDebounceTime = millis();
  } else if(pedestrianButtonState == LOW && isPressingPedestrianButton) {
    if(millis() - PedestrianButtonlastDebounceTime > debounceDelay) {
      isPressingPedestrianButton = NOT_PRESSING_BUTTON;
      if(status_light_pedestrian == RED_LIGHT) { //Only when pedestrians are waiting
        pedestrian_button_pressed = PRESSING_BUTTON;
      }
    }
  }
}

void turnMyselfRed() {
  if(status_light_incoming == NO_LIGHTS) {
    turnLightsOff(); //First time
    //Incoming traffict light
    digitalWrite(INCOMING_RED_PIN, HIGH);
    status_light_incoming = RED_LIGHT;
    //Interior traffic light
    digitalWrite(INTERIOR_GREEN_PIN, HIGH);
    status_light_inside = GREEN_LIGHT;
    //Pedestrian traffic light
    digitalWrite(PEDESTRIAN_GREEN_PIN, HIGH);
    status_light_pedestrian = GREEN_LIGHT;
  } else {
    //Incoming traffic light
    digitalWrite(INCOMING_GREEN_PIN, LOW);
    digitalWrite(INCOMING_YELLOW_PIN, HIGH);
    delay(DELAY_YELLOW_BLINK); //Yellow for 0.5 secs
    digitalWrite(INCOMING_YELLOW_PIN, LOW);
    digitalWrite(INCOMING_RED_PIN, HIGH);
    status_light_incoming = RED_LIGHT;

    //Turn pedestrian light green 
    digitalWrite(PEDESTRIAN_GREEN_PIN, HIGH);
    status_light_pedestrian = GREEN_LIGHT;

    //Turn inside traffic light also green
    digitalWrite(INTERIOR_RED_PIN, LOW);
    digitalWrite(INTERIOR_YELLOW_PIN, HIGH);
    delay(DELAY_YELLOW_BLINK); //Yellow for 0.5 secs
    digitalWrite(INTERIOR_YELLOW_PIN, LOW);
    digitalWrite(INTERIOR_GREEN_PIN, HIGH);
    status_light_inside = GREEN_LIGHT;

  }
}
void turnMyselfGreen() {
  if(status_light_inside == NO_LIGHTS) {
    turnLightsOff(); //First time
    //Interior traffic light
    digitalWrite(INTERIOR_RED_PIN, HIGH);
    status_light_inside = RED_LIGHT;
    //Pedestrian traffic light
    digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);
    status_light_pedestrian = RED_LIGHT;
    //Incoming traffict light
    digitalWrite(INCOMING_GREEN_PIN, HIGH);
    status_light_incoming = GREEN_LIGHT;    
  } else {
    //Turn pedestrian light red 
    digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);
    status_light_pedestrian = RED_LIGHT;

    //Turn inside traffic light also red
    digitalWrite(INTERIOR_GREEN_PIN, LOW);
    digitalWrite(INTERIOR_YELLOW_PIN, HIGH);
    delay(DELAY_YELLOW_BLINK); //Yellow for 0.5 secs
    digitalWrite(INTERIOR_YELLOW_PIN, LOW);
    digitalWrite(INTERIOR_RED_PIN, HIGH);
    status_light_inside = RED_LIGHT;

    //Incoming traffic light
    digitalWrite(INCOMING_RED_PIN, LOW);
    digitalWrite(INCOMING_YELLOW_PIN, HIGH);
    delay(DELAY_YELLOW_BLINK); //Yellow for 0.5 secs
    digitalWrite(INCOMING_YELLOW_PIN, LOW);
    digitalWrite(INCOMING_GREEN_PIN, HIGH);
    status_light_incoming = GREEN_LIGHT;

    

    

  }
}

void turnLightsOff() {
  digitalWrite(INTERIOR_RED_PIN, LOW);
  digitalWrite(INTERIOR_YELLOW_PIN, LOW);
  digitalWrite(INTERIOR_GREEN_PIN, LOW);
  digitalWrite(INCOMING_RED_PIN, LOW);
  digitalWrite(INCOMING_YELLOW_PIN, LOW);
  digitalWrite(INCOMING_GREEN_PIN, LOW);
  digitalWrite(PEDESTRIAN_GREEN_PIN, LOW);
  status_light_incoming = NO_LIGHTS;
  status_light_inside = NO_LIGHTS;
  status_light_pedestrian = NO_LIGHTS;
}

void initiateTrafficLightIfNeeded() {
  //The first time a RED or GREEN commands is received
  if(blinking) {
    blinking = BLINKING_OFF;
    turnLightsOff();
  }
}

void handleOperation(int opNumber) {
  pedestrian_button_pressed = NOT_PRESSING_BUTTON;
  switch(opNumber) {
    case RED_CMD:
      initiateTrafficLightIfNeeded();
      turnMyselfRed();
      request_received = NOTHING;
      break;
    case GREEN_CMD:
      initiateTrafficLightIfNeeded();
      turnMyselfGreen();
      request_received = NOTHING;
      break;
    case OFF_CMD:
      turnLightsOff();
      blinking = BLINKING_ON;
      request_received = NOTHING;
      break;
  }
}

void messageReceivedHandler() {
  lastTimeMessageWasReceived = millis();
  int sender, opNumber, destination, integrity;
  while(4 <= Wire.available()) {
    // Each command has 4 bytes
    sender = Wire.read();
    opNumber = Wire.read();
    destination = Wire.read();
    integrity = Wire.read();
    if(integrity == (sender + opNumber + destination)) {
      request_received = opNumber;
      Serial.print("Op received: ");
      Serial.println(opNumber);
    } else {
      Serial.println("Integrity mismatch when receiving a message.");
    }

  }
}

void requestReceivedHandler() {
  //The orchestratos asks for an answer after a command
  if(request_received == PING_CMD) {
    Serial.println("Answering to PING");
    char response [FIVE_BYTES];
    unsigned char information = 0; //0 means no failures.
    //We cant check if a light is failing because we send digitalWrite commands but we dont get an answer
    information = (pedestrian_button_pressed ? 2 : 0); //2 equals to a binary 10. Therefore the 7th bit is affected.

    if (!sprintf(response, "%c%c%c%c%c", localAddress, STATUS_CMD, 0, information, localAddress+STATUS_CMD+information)) { //0 is orchestrator
      Serial.println("BROKEN SPRINTF");
    } else {
      Wire.write(response, FIVE_BYTES);
    }
    request_received = NOTHING;
  } else if(request_received != NOTHING) {
    char response [FOUR_BYTES];
    if (!sprintf(response, "%c%c%c%c", localAddress, ACK, 0, ACK+localAddress)) { //0 is orchestrator
      Serial.println("BROKEN SPRINTF");
    } else {
      Wire.write(response, FOUR_BYTES);
    }
  }
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
      Serial.println(int(status_light_inside));
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
