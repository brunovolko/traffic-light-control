#include <Wire.h>
#include <stdio.h>
#include "master.h"
#include "slave.h"
#include "constants.h"




char systemStatus = SystemOFF; //The system initializes off
char localAddress; //from 0 to 3, depends which master I am

char slaves = 1, isPressingOnButton = 0; //number of slaves




void setup() {
  Serial.begin(9600); //For debug
  pinMode(A0,INPUT); //For address
  pinMode(A1,INPUT); //For address
  pinMode(ON_OFF_BUTTON_PIN, INPUT); //On/off button
  pinMode(PotentiometerPin, INPUT); //Potentiometer
  
  delay(TIME_DELAY_TO_CHECK_ADDRESS); //TODO check
  getAddress();
  if(localAddress == 0) {
    Wire.begin();
    Serial.println("I am master");
    //TODO check how many slaves are there
  }
}

void getAddress(){
  char firstBit = analogRead(A0) > 100 ? 1 : 0;
  char secondBit = analogRead(A1) > 100 ? 1 : 0;
  localAddress = firstBit + 2 * secondBit;
}




void loop() {
  if (localAddress == 0) {
    checkOnButton(); //1st check for on button
    if(systemStatus) {
      handlePotentiometer(); //2nd chandle potentiometer
      //3rd check pedestrian button
      //4th act accordingly
      }
    for(int i = 0; i < slaves; i++) {
      char opNumber = 0;//TODO get the actual operation number
      sendMessage(opNumber, i+1);
    }
  }

  if(systemStatus == 0) {
    blink();
  }
}
