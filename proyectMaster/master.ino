#include "master.h"
#include "constants.h"

int timeIntervalToBlink = 0;

void checkOnButton(){
  char onButtonState = digitalRead(ON_OFF_BUTTON_PIN);
  if(onButtonState == HIGH && !isPressingOnButton) {
    isPressingOnButton = 1;
  } else if(onButtonState == LOW && isPressingOnButton) {
    isPressingOnButton = 0;
    systemStatus = !systemStatus;
  }  
}

void handlePotentiometer(){
  int potentiometerStatus = analogRead(PotentiometerPin);
  timeIntervalToBlink = 2000 + potentiometerStatus / 1024 * 13000;
}


void sendMessage(char opNumber, char destination) { //communicate with light and recieve the answer from it
  char sender = 0;
  char integrity = sender + opNumber + destination;

  char toSend[4];
  if (!sprintf(toSend, "%c%c%c%c",sender, opNumber, destination, integrity)) {
    Serial.println("BROKEN SPRINTF");  
  }
  if(destination == localAddress) {
    //sendAndRecieveLocalMessage
  } else {
    Wire.beginTransmission(destination);
    Wire.write(toSend);  
    Wire.endTransmission();
    Wire.requestFrom(destination, opNumber == PING ? 4 : 5);
    while(Wire.available()){
      char c = Wire.read();
      Serial.print(c);
      //TODO
    }
  }
}
