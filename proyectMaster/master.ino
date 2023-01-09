#include "master.h"
#include "constants.h"

long timeIntervalForNextEntry = 0; //Time to wait to change from one entry to the next one
unsigned long OnButtonlastDebounceTime = 0;  // the last time the output pin was toggled

void setupMaster() {
  pinMode(ADDRESS_PIN_0, INPUT); //For address
  pinMode(ADDRESS_PIN_1, INPUT); //For address
  pinMode(ON_OFF_BUTTON_PIN, INPUT); //On/off button
  pinMode(POTENTIOMETER_PIN, INPUT); //Potentiometer
  pinMode(STATUS_LED_PIN, OUTPUT); //Status led
}

void checkOnButton() {
  //Verifies if the on/off button experienced OFF -> ON -> OFF
  int onButtonState = digitalRead(ON_OFF_BUTTON_PIN);  
  if(onButtonState == HIGH && !isPressingOnButton) {
    isPressingOnButton = PRESSING_BUTTON;
    OnButtonlastDebounceTime = millis();
  } else if(onButtonState == LOW && isPressingOnButton) {
    if(millis() - OnButtonlastDebounceTime > debounceDelay) {
      Serial.println("Changing system status");
      isPressingOnButton = NOT_PRESSING_BUTTON;
      blinking = !blinking; //Toggles system status
      digitalWrite(STATUS_LED_PIN, (blinking ? LOW : HIGH)); //Change the status led
      if(blinking) {
        turnSlavesOff(); //Start to blink and commands slaves to blink
        lastTrafficLightUpdate = 0; //Starts from initial state
      } else {
        blinking = BLINKING_OFF;
      }
      status_light_incoming = NO_LIGHTS;
      status_light_inside = NO_LIGHTS;
      status_light_pedestrian = NO_LIGHTS;
        

    }
  }  
}

void failureDetected(char * reason, bool turnOthersOff) {
  Serial.print("FAILURE DETECTED. ");
  Serial.println(reason);
  if(turnOthersOff)
    turnSlavesOff(); //In case of failure every traffic light blinks
  lastTrafficLightUpdate = 0;
}

void sendLightChangeToTrafficLight(int entry, int opNumber) {
  if(entry == 0) { //For commands to the same orchestrator
    if(opNumber == GREEN_CMD)
      turnMyselfGreen();
    else
      turnMyselfRed();
  } else {
    int bytes_read;
    for(int i = 0; i<3; i++) { //Up to 3 attempts in case of ACK errors
      bytes_read = sendMessage(opNumber, entry); //Send OFF command through WIRE
      if(bytes_read != -1)
        break;
      delay(100); //In case of failure
    }
    if(bytes_read == -1)
      failureDetected("Light change", true);
  }
}

void turnGreen(int entry) {
  //The whole process of changing to a new entry
  if(lastTrafficLightUpdate == 0) { //Only the first cycle
    //First we turn all red except the first one.
    for(int i = 1; i < 4; i++) {
      if(slavesAvailable[i] == SLAVE_ACTIVE) {
        sendLightChangeToTrafficLight(i, RED_CMD);
      }
    }
    //Turn the first traffic light green
    sendLightChangeToTrafficLight(0, GREEN_CMD);
  } else {
    
    int prevEntry = entry;
    while(true) {
      prevEntry = (prevEntry > 0 ? (prevEntry-1) % 4 : 3); //First we turn red the previous traffic light
      if(slavesAvailable[prevEntry] == SLAVE_ACTIVE)
        break;
    }

    //First turn red the previous entry
    sendLightChangeToTrafficLight(prevEntry, RED_CMD);

    if(!blinking) { //An error might have happened before
      //Now we turn green the new entry
      sendLightChangeToTrafficLight(entry, GREEN_CMD);
    }
    

  }
}

void orchestrate() {
  long current = millis();
  if(lastTrafficLightUpdate == 0) { //First time
    turnGreen(0); //Start with first entry
    lastTrafficLightUpdate = current;
  } else if(current - lastTrafficLightUpdate > timeIntervalForNextEntry) {
    remainingTimeHalved = false; //New cycle, therefore nobody pressed the pedestrian button
    pedestrian_button_pressed = NOT_PRESSING_BUTTON;
    while(true) { //Calculate the next entry to switch
      currentEntry = (currentEntry+1) % 4;
      if(slavesAvailable[currentEntry] == SLAVE_ACTIVE)
        break;
    }
    
    turnGreen(currentEntry);
    lastTrafficLightUpdate = millis();
  }
}

void askIfPedestrianButtonIsPressed() {
  //A ping message is sent
  if(currentEntry != 0) {
    int bytesRead;
    for(int i = 0; i < 3; i++) { //3 attempts in case of an ACK error
      bytesRead = sendMessage(PING_CMD, currentEntry); //Asking the current entry if pedestrian button is pressed
      if(bytesRead != -1)
        break;
    }
    if(bytesRead == -1)
      failureDetected("Failed PING", true);
  } else {
    if(pedestrian_button_pressed) { //In case the orchestrator pedestrian button has been pressed
      Serial.println("My own button is pressed");
      halveRemainingTime();
    }
  }
  
    
}

void handlePotentiometer(){
  long potentiometerStatus = analogRead(POTENTIOMETER_PIN);
  timeIntervalForNextEntry = 2000 + potentiometerStatus * 13000 / 1024 ;
}


int sendMessage(char opNumber, char destination) {
  int bytes_to_expect = (opNumber == PING_CMD ? FIVE_BYTES : FOUR_BYTES);

  char response [bytes_to_expect];
  char sender = 0;
  char integrity = sender + opNumber + destination;

  char toSend[4];
  if (!sprintf(toSend, "%c%c%c%c",sender, opNumber, destination, integrity)) {
    Serial.println("BROKEN SPRINTF");
  }
  
  Wire.beginTransmission(destination);
  Wire.write(toSend, 4);
  Wire.endTransmission();

  
  int bytes_read = 0;
  
  Wire.requestFrom(destination, bytes_to_expect);
  while(Wire.available()){
    char c = Wire.read();
    response[bytes_read++] = c;
  }
  if(bytes_to_expect == FOUR_BYTES) {
    if(!verifyAck(response,destination)) {
      Serial.println("ACK FAILURE");
      Serial.println(int(response[0]));
      Serial.println(int(response[1]));
      Serial.println(int(response[2]));
      Serial.println(int(response[3]));
      bytes_read = -1; //Failure
    }
  } else {
    //After a PING we receive a status
    if(bytes_read != bytes_to_expect || !verifyStatus(response,destination))
      bytes_read = -1; //Failure
  }
  blink_status_led = 1;
  digitalWrite(STATUS_LED_PIN, LOW);
  last_status_led_blink = millis();
  return bytes_read;
}

void halveRemainingTime() {
  long remainingTime = timeIntervalForNextEntry - (millis() - lastTrafficLightUpdate);
  remainingTime /= 2;
  timeIntervalForNextEntry = (millis() - lastTrafficLightUpdate) + remainingTime;
  remainingTimeHalved = true;
}

bool verifyStatus(char * response, int address) {
  if(int(response[0]) == address && response[1] == STATUS_CMD && response[2] == 0 && response[4] == (address + response[3] + STATUS_CMD)) {
    if(response[3] == 2) {
      //Button pressed
      halveRemainingTime();
    }
    return true;
  }
  return false;

}

bool verifyAck(char * response, int address) {
  return response[0] == address && response[1] == ACK && response[2] == 0 && response[3] == (address+ACK);
}

void turnSlavesOff() {
  blinking = BLINKING_ON; //For traffic light 0
  digitalWrite(STATUS_LED_PIN, LOW); //Blue led off
  turnLightsOff(); //For traffic light 0

  
  for(int i = 1; i < 4; i++) {
    if(slavesAvailable[i] == SLAVE_ACTIVE) {
      int bytes_read = sendMessage(OFF_CMD, i); //Send OFF command through WIRE
      if(bytes_read == -1)
        failureDetected("turn slaves off", false);
    }
  }
}

void detectSlaves() {
  slavesAvailable[0] = SLAVE_ACTIVE; //This is the traffic light sharing controller with the orchestrator
  int bytesRead;
  for(int i = 1; i < 4; i++) { //Iterate over the other 3 possible slaves
    bytesRead = sendMessage(PING_CMD, i); //Checking if controller i exists
    slavesAvailable[i] = (bytesRead <= 0 ? SLAVE_INACTIVE : SLAVE_ACTIVE);
  }

}
