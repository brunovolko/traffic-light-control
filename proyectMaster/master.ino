#include "master.h"
#include "constants.h"

long timeIntervalForNextEntry = 0; //Blink? or change lights? TODO

unsigned long OnButtonlastDebounceTime = 0;  // the last time the output pin was toggled

void setupMaster() {
  pinMode(ADDRESS_PIN_0, INPUT); //For address
  pinMode(ADDRESS_PIN_1, INPUT); //For address
  pinMode(ON_OFF_BUTTON_PIN, INPUT); //On/off button
  pinMode(POTENTIOMETER_PIN, INPUT); //Potentiometer
  pinMode(STATUS_LED_PIN, OUTPUT); //Status led
}

void checkOnButton(){
  int onButtonState = digitalRead(ON_OFF_BUTTON_PIN);  
  if(onButtonState == HIGH && !isPressingOnButton) {
    isPressingOnButton = PRESSING_BUTTON;
    OnButtonlastDebounceTime = millis();
  } else if(onButtonState == LOW && isPressingOnButton) {
    if(millis() - OnButtonlastDebounceTime > debounceDelay) {
      isPressingOnButton = NOT_PRESSING_BUTTON;
      systemStatus = !systemStatus;
      digitalWrite(STATUS_LED_PIN, (systemStatus ? HIGH : LOW)); //Change the status led
    }
  }  
}

void handlePotentiometer(){
  long potentiometerStatus = analogRead(POTENTIOMETER_PIN);
  timeIntervalForNextEntry = 2000 + potentiometerStatus * 13000 / 1024 ;
  Serial.println(timeIntervalForNextEntry);
}


int sendMessage(char opNumber, char destination, char * response) { //communicate with light and recieve the answer from it\
  //TODO ver mas adelante si el response es necesario o no
  char sender = 0;
  char integrity = sender + opNumber + destination;

  char toSend[4];
  if (!sprintf(toSend, "%c%c%c%c",sender, opNumber, destination, integrity)) {
    Serial.println("BROKEN SPRINTF");
  }
  if(destination == localAddress) {
    //TODO sendAndRecieveLocalMessage
  } else {
    Wire.beginTransmission(destination);
    Wire.write(toSend, 4);
    Wire.endTransmission();

    char bytes_to_expect = (opNumber == PING_CMD ? FIVE_BYTES : FOUR_BYTES);
    int bytes_read = 0;
    
    Wire.requestFrom(destination, bytes_to_expect);
    while(Wire.available()){
      char c = Wire.read();
      response[bytes_read++] = c;
    }
    return bytes_read;
  }
}

bool verifyAck(char * response, int address) {
  return response[0] == address && response[1] == ACK && response[2] == 0 && response[3] == (address+4);
}

void turnSlavesOff() {
  blinking = BLINKING_ON; //For traffic light 0
  turnLightsOff(); //For traffic light 0

  char response[FOUR_BYTES];  
  for(int i = 1; i < 4; i++) {
    if(slavesAvailable[i] == SLAVE_ACTIVE) {
      sendMessage(i, OFF_CMD, response); //Send OFF command through WIRE
      if(!verifyAck(response, i)) //Make sure the response was ACK
        Serial.println("ACK FAILED"); // TODO handle failure
    }
    
  }
}

void detectSlaves() {
  slavesAvailable[0] = SLAVE_ACTIVE; //This is the traffic light sharing controller with the orchestrator
  char response[FIVE_BYTES]; //To store the response of each message
  int bytesRead;
  for(int i = 1; i < 4; i++) { //Iterate over the other 3 possible slaves
    bytesRead = sendMessage(PING_CMD, i, response); //Checking if controller i exists
    slavesAvailable[i] = (bytesRead == 0 ? SLAVE_INACTIVE : SLAVE_ACTIVE);
  }

}
