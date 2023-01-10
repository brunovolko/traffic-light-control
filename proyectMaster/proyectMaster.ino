#include <Wire.h>
#include <stdio.h>
#include "master.h" //Functions and variables related to the orchestrator
#include "slave.h" //Functions and variables related to the slave/traffic light
#include "constants.h"




int localAddress; //My own identifier. From 0 to 3.
char isPressingOnButton = NOT_PRESSING_BUTTON;
bool blink_status_led; //Used to know if loop should make the status led blink
long last_status_led_blink = 0; //Used to avoid delay function when blinking status led
long lastTimeAskedPedestrianButtonStatus = 0; //To avoid asking for the pedestratian button status so often.
long lastTimeMessageWasReceived = 0; //For fault tolerance



void setup() {
  Serial.begin(9600); //For debug
  
  setupMaster();
  setupSlave();

  
  getAddress(); //It identifies my own address based on the wires position
  if(localAddress == 0) {
    Wire.begin();
    Serial.println("I am the orchestrator");
    digitalWrite(STATUS_LED_PIN, (blinking ? LOW : HIGH)); //Make sure the led starts with the initial status
    detectSlaves(); //Since we dont know how many slaves are there, we send a PING to them to see if they are alive
    turnSlavesOff(); //Send OFF command to slaves
  } else {
    lastTimeMessageWasReceived = millis();
    Wire.begin(localAddress);
    Serial.print("I'm slave N");
    Serial.println(localAddress);

    Wire.onReceive(messageReceivedHandler);
    Wire.onRequest(requestReceivedHandler);
  }
}

void getAddress(){
  int firstBit = digitalRead(ADDRESS_PIN_0) == HIGH ? 1 : 0;
  int secondBit = digitalRead(ADDRESS_PIN_1) == HIGH ? 1 : 0;
  localAddress = firstBit + 2 * secondBit;
}




void loop() {
  if (localAddress == 0) {
    checkOnButton(); //1st check status of the on/off button
    if(blinking == BLINKING_OFF) { //It means the system is ON
      if(!remainingTimeHalved) //Nobody has pressed the pedestrian button yet
        handlePotentiometer(); //Read potentiometer to adjust the time of the traffic lights
      
      if(blink_status_led) { //It makes the status led blink when it has to
        if(millis() - last_status_led_blink > TIME_BLINK_STATUS_LED) {
          digitalWrite(STATUS_LED_PIN, HIGH);
          blink_status_led = 0;
        }
      }

      orchestrate(); //It sends orchestration commands when its time
      if(!remainingTimeHalved && (millis() - lastTimeAskedPedestrianButtonStatus > 1000)) { //Every 1 second
        askIfPedestrianButtonIsPressed();
        lastTimeAskedPedestrianButtonStatus = millis();
      }
      
    }
    
  } else {
    if(request_received != NOTHING) {
      Serial.println(request_received);
      handleOperation(request_received); //Execute the operation received through message
    }
    request_received = NOTHING;
    if(!blinking && (millis()-lastTimeMessageWasReceived > 10000)) {
      Serial.println("No message received after a minute");
      turnLightsOff();
      blinking = BLINKING_ON;

    }
  } 


  if(blinking == BLINKING_ON) {
    blink();
  } else {
    checkPedestrianButton(); //The slaves checks if someone is pressing the button
  }
}
