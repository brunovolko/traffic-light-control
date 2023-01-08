#include <Wire.h>
#include <stdio.h>
#include "master.h" //Functions and variables related to the orchestrator
#include "slave.h" //Functions and variables related to the slave/traffic light
#include "constants.h"




char systemStatus = SystemOFF; //The system initializes off
int localAddress; //My identifier. From 0 to 3.
char isPressingOnButton = NOT_PRESSING_BUTTON;




void setup() {
  Serial.begin(9600); //For debug
  
  setupMaster();
  setupSlave();

  
  delay(TIME_DELAY_TO_CHECK_ADDRESS);
  getAddress();
  if(localAddress == 0) {
    Wire.begin();
    Serial.println("I am the orchestrator");
    digitalWrite(STATUS_LED_PIN, (systemStatus ? HIGH : LOW)); //Make sure the led starts with the initial status
    detectSlaves(); //Since we dont know how many slaves are there, we send a PING to them to see if they are alive
    turnSlavesOff(); //Send OFF command to slaves
  } else {
    Serial.print("I'm slave N");
    Serial.println(localAddress);

    Wire.onReceive(messageReceivedHandler);
  }
}

void getAddress(){
  int firstBit = digitalRead(ADDRESS_PIN_0) == HIGH ? 1 : 0;
  int secondBit = digitalRead(ADDRESS_PIN_1) == HIGH ? 1 : 0;
  localAddress = firstBit + 2 * secondBit;
}




void loop() {
  if (localAddress == 0) {
    checkOnButton(); //1st check for on button
    if(systemStatus == SystemON) {
      handlePotentiometer(); //Read potentiometer to adjust the time of the traffic lights
      //TODO 3rd check pedestrian button
      //TODO orchestrate lights
      //TODO detect faults
      //TODO turn on or off the status led
      //TODO show patterns in status led
      //TODO handle pedestrian button time changes
      

      //for(int i = 0; i < slaves; i++) {
        //char opNumber = 0;//TODO get the actual operation number
        //sendMessage(opNumber, i+1);
      //}
    }
    
  }

  //TODO handle commands received as a slave

  if(blinking == BLINKING_ON) {
    blink();
  }
}
