#include "PS2X_lib.h"  //for v1.6

/******************************************************************
 * set pins connected to PS2 controller:
 *   - 1e column: original 
 *   - 2e colmun: Stef?
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        12  //13    
#define PS2_CMD        11  //11
#define PS2_SEL        10  //10
#define PS2_CLK        13
//12

//#define DEBUG
//#define Serial.print() //
/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
#define pressures   false //or true
#define rumble      false //or true

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

char left[] = {0xff, 0x55, 0x07, 0x00, 0x02, 0x05, 0x9c, 0xff, 0x64, 00};
char right[] = {0xff, 0x55, 0x07, 0x00, 0x02, 0x05, 0x9c, 0xff, 0x9c, 0xff};

char right_wheel_fw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 9, 0x3F, 0xFF, 0, 0, 0, 0xA};
char left_wheel_fw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 10, 0xCF, 0x0, 0, 0, 0, 0xA};

char right_wheel_bw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 9, 0xCF, 0x0, 0, 0, 0, 0xA};
char left_wheel_bw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 10, 0x3f, 0xFF, 0, 0, 0, 0xA};

char right_wheel_stop[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 9, 0x0, 0x0, 0, 0, 0, 0xA};
char left_wheel_stop[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 10, 0xFF, 0xFF, 0, 0, 0, 0xA};

char stop[] = {0xff, 0x55, 0x07, 00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00};

char LED[] = {0xFF, 0x55, 0x9, 0, 0x2, 0x8, 0x7, 0x2, 0, 0x20, 0x20, 0x20, 0xA};
char BUZZER[] = {0xFF, 0x55, 0x9, 0, 0x2, 0x22, 0x26,0x1, 0xFA, 0,0,0, 0xA};

void setup(){
 
  Serial.begin(115200);
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
//  type = ps2x.readType(); 
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
//  if(error == 1) //skip loop if no controller found
//    return; 

 //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed

    if(ps2x.Button(PSB_PAD_UP)) {      //will be TRUE as long as button is pressed
      Serial.print("Up held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_UP), DEC);
        for(int i=0; i<sizeof(left_wheel_fw); i++) {
            Serial.write(left_wheel_fw[i]);
        }
        for(int i=0; i<sizeof(left); i++) {
            Serial.write(right_wheel_fw[i]);
        }
    }
    if(ps2x.Button(PSB_PAD_RIGHT)){
      for(int i=0; i<sizeof(right_wheel_fw); i++) {
        Serial.write(right_wheel_fw[i]);
      }
      for(int i=0; i<sizeof(left_wheel_stop); i++) {
        Serial.write(left_wheel_stop[i]);
      }
    }
    if(ps2x.Button(PSB_PAD_LEFT)){
      for(int i=0; i<sizeof(left_wheel_fw); i++) {
        Serial.write(left_wheel_fw[i]);
      }
      for(int i=0; i<sizeof(right_wheel_stop); i++) {
        Serial.write(right_wheel_stop[i]);
      }
    }
    if(ps2x.Button(PSB_PAD_DOWN)){
      for(int i=0; i<sizeof(left_wheel_fw); i++) {
          Serial.write(left_wheel_bw[i]);
      }
      for(int i=0; i<sizeof(left); i++) {
          Serial.write(right_wheel_bw[i]);
      }
    }   
//    vibrate = ps2x.Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button
    if(ps2x.ButtonPressed(PSB_CIRCLE)) {               //will be TRUE if button was JUST pressed
      Serial.println("Circle just pressed");
        for(int i=0; i<sizeof(left); i++) {
          Serial.write(stop[i]);
        }
    }

    if(ps2x.ButtonPressed(PSB_CROSS)) {               //will be TRUE if button was JUST pressed
//      Serial.println("Circle just pressed");
        for(int i=0; i<sizeof(LED); i++) {
          Serial.write(LED[i]);
        }   
    }
    if(ps2x.ButtonPressed(PSB_SQUARE)) {               //will be TRUE if button was JUST pressed
        for(int i=0; i<sizeof(BUZZER); i++) {
          Serial.write(BUZZER[i]);
        }   
    }    
  delay(50);
  
  for(int i=0; i<sizeof(left); i++) {
     Serial.write(stop[i]);
  }  
}
