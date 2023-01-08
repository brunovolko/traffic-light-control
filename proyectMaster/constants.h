#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_



#define SystemON 1
#define SystemOFF 0
#define BLINKING_OFF 0
#define BLINKING_ON 1
#define PRESSING_BUTTON 1
#define NOT_PRESSING_BUTTON 0
#define DELAY_YELLOW_BLINK 500 //500ms
#define FOUR_BYTES 4
#define FIVE_BYTES 5

#define debounceDelay 50 // Used to debounce buttons

#define ADDRESS_PIN_0 2
#define ADDRESS_PIN_1 3


enum Operation {RED_CMD, GREEN_CMD, OFF_CMD, PING_CMD, ACK, STATUS_CMD, NOTHING};
enum LightStatus {NO_LIGHTS, RED_LIGHT, GREEN_LIGHT, YELLOW_LIGHT};

#endif