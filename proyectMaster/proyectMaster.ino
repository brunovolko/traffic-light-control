#include <Wire.h>
#include <stdio.h>
#include "master.h" //Functions and variables related to the orchestrator
#include "slave.h" //Functions and variables related to the slave/traffic light
#include "constants.h"




int localAddress; //My identifier. From 0 to 3.
char isPressingOnButton = NOT_PRESSING_BUTTON;
bool blink_status_led;
long last_status_led_blink = 0;




void setup() {
  Serial.begin(9600); //For debug
  
  setupMaster();
  setupSlave();

  
  delay(TIME_DELAY_TO_CHECK_ADDRESS);
  getAddress();
  if(localAddress == 0) {
    Wire.begin();
    Serial.println("I am the orchestrator");
    digitalWrite(STATUS_LED_PIN, (blinking ? LOW : HIGH)); //Make sure the led starts with the initial status
    detectSlaves(); //Since we dont know how many slaves are there, we send a PING to them to see if they are alive
    turnSlavesOff(); //Send OFF command to slaves
  } else {
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
    checkOnButton(); //1st check for on button
    if(blinking == BLINKING_OFF) {
      handlePotentiometer(); //Read potentiometer to adjust the time of the traffic lights
      //TODO 3rd check pedestrian button
      //TODO detect faults
      //TODO show patterns in status led
      //TODO handle pedestrian button time changes
      if(blink_status_led) {
        if(millis() - last_status_led_blink > TIME_BLINK_STATUS_LED) {
          digitalWrite(STATUS_LED_PIN, HIGH);
          blink_status_led = 0;
        }
      }

      orchestrate();
    }
    
  } else {
    if(request_received != NOTHING) {
      Serial.println(request_received);
      handleOperation(request_received);
    }
    request_received = NOTHING;
  }


  if(blinking == BLINKING_ON) {
    blink();
  }
}
