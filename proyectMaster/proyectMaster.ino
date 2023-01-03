#include <Wire.h>
#include <stdio.h>
#include "master.h"
#include "slave.h"
#include "constants.h"




char systemStatus = SystemOFF; //The system initializes off
int localAddress; //from 0 to 3, depends which master I am

char isPressingOnButton = 0; //number of slaves




void setup() {
  Serial.begin(9600); //For debug
  
  setupMaster();
  setupSlave();

  
  //delay(TIME_DELAY_TO_CHECK_ADDRESS); //TODO check
  getAddress();
  if(localAddress == 0) {
    Wire.begin();
    Serial.println("I am the orchestrator");
    detectSlaves(); //Since we dont know how many slaves are there, we send a PING to them to see if they are alive
    turnSlavesOff();
  }
}

void getAddress(){
  int firstBit = digitalRead(ADDRESS_PIN_0) == HIGH ? 1 : 0;
  int secondBit = digitalRead(ADDRESS_PIN_1) == HIGH ? 1 : 0;
  localAddress = firstBit + 2 * secondBit;
}




void loop() {
  return;
  if (localAddress == 0) {
    checkOnButton(); //1st check for on button
    if(systemStatus == SystemON) {
      handlePotentiometer(); //2nd chandle potentiometer
      //3rd check pedestrian button
      //4th act accordingly

      //for(int i = 0; i < slaves; i++) {
        //char opNumber = 0;//TODO get the actual operation number
        //sendMessage(opNumber, i+1);
      //}
    }
    
  }

  if(blinking == BLINKING_ON) {
    blink();
  }
}
