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
byte vibrate = 0;
signed char pss_lx = 0;
signed char pss_ly = 0;
char need_stop = 0;
char LED_state = 0;

/*
   All the instruction format are template
   We should change the key byte before we use it.
*/
char right_wheel_fw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 9, 0xeF, 0xFF, 0, 0, 0, 0xA};
char left_wheel_fw[] = {0xFF, 0x55, 0x9, 0, 0x2, 0xA, 10, 0xeF, 0x0, 0, 0, 0, 0xA};

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
}

// The 7th byte is speed of motor
void setspeed_one_wheel (char cmd_wheel[], char speed) {
	cmd_wheel[7] = speed;
}

// Notice, the second byte describe the len of the whole instruction.
// The instruction should be cmd[2]+4, the extra 4 byte are:
// 0xFF, 0x55, <length>, and the last byte 0xA
void write_to_wheel (char cmd_wheel[]) {
        for(int i=0; i < cmd_wheel[2]+4; i++) {
            Serial.write(cmd_wheel[i]);
        }
}

void set_led(char state) {
	switch(state) {
	case 0:
		LED[9] = 60;
		LED[10] = 0;
		LED[11] = 0;
		break;
	case 1:
		LED[9] = 0;
		LED[10] = 60;
		LED[11] = 0;
		break;
	case 2:
		LED[9] = 0;
		LED[10] = 0;
		LED[11] = 60;
		break;
	}

        for(int i=0; i<sizeof(LED); i++) {
          Serial.write(LED[i]);
        }   
}

/* direct=1 means forward.
   Others means backward. */
void move (char speed, char direct = 1) {
	need_stop = 1;
	if (direct == 1) {
		setspeed_one_wheel(left_wheel_fw, speed);
		setspeed_one_wheel(right_wheel_fw, 255 - speed);

		write_to_wheel(left_wheel_fw);
		write_to_wheel(right_wheel_fw);
	} else {
		setspeed_one_wheel(left_wheel_bw, 255 - speed);
		setspeed_one_wheel(right_wheel_bw, speed);

		write_to_wheel(left_wheel_bw);
		write_to_wheel(right_wheel_bw);
	}
}

/* The X and Y should be from game joystick, the axis X and asis Y. */
void moveXY (signed char speed_x, signed char speed_y) {
	need_stop = 1;
	unsigned char speed_l, speed_r = 0;
	signed char direct = -speed_y / abs(speed_y);   //1: fw  -1: bw
	signed int tmp = 0;
	float stress;

	if (speed_y > 127) speed_y = 127;
	if (speed_y < -127) speed_y = -127;

	stress = sqrt(speed_x * speed_x + speed_y * speed_y);
	if (stress > 127) stress = 127;

	speed_l = speed_r = (unsigned char) stress * 2;

	if (speed_x > 0) {
		tmp = speed_l;
		tmp -= 2*speed_x;
		speed_l = tmp < 0 ? 0 : (unsigned char)tmp;

	} else {
		tmp = speed_r;
		tmp += 2*speed_x;

		speed_r = tmp < 0 ? 0 : (unsigned char)tmp;
	}

	if(direct == 1) {
		setspeed_one_wheel(left_wheel_fw, speed_l);
		setspeed_one_wheel(right_wheel_fw, 255 - speed_r);

		write_to_wheel(left_wheel_fw);
		write_to_wheel(right_wheel_fw);
	} else if ( direct == -1 ) {
		setspeed_one_wheel(left_wheel_bw, 255 - speed_l);
		setspeed_one_wheel(right_wheel_bw, speed_r);

		write_to_wheel(left_wheel_bw);
		write_to_wheel(right_wheel_bw);
	}
}

void loop() {
 //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed

    if(ps2x.Button(PSB_PAD_UP)) {      //will be TRUE as long as button is pressed
	move(200);
    }
    if(ps2x.Button(PSB_PAD_RIGHT)){
	moveXY(80,-100);
    }
    if(ps2x.Button(PSB_PAD_LEFT)){
	moveXY(-80,-100);
    }
    if(ps2x.Button(PSB_PAD_DOWN)){
	move(200, 0);
    }   
    if(ps2x.ButtonPressed(PSB_CIRCLE)) {               //will be TRUE if button was JUST pressed
      Serial.println("Circle just pressed");
        for(int i=0; i<sizeof(stop); i++) {
          Serial.write(stop[i]);
        }
    }

    if(ps2x.ButtonPressed(PSB_CROSS)) {               //will be TRUE if button was JUST pressed
	set_led(LED_state % 3);
	LED_state++;
    }

    if(ps2x.ButtonPressed(PSB_TRIANGLE)) {
        Serial.println("TRIANGLE just pressed");
        for(int i=0; i<sizeof(BUZZER); i++) {
          Serial.write(BUZZER[i]);
        }   
    }    

    unsigned int tmp = ps2x.Analog(PSS_LX);
    tmp = tmp == 0? -127 : tmp - 128;    //make sure tmp is [-127, 127]
    pss_lx = (unsigned char)tmp; 

    tmp = ps2x.Analog(PSS_LY);
    tmp = tmp == 0? -127 : tmp - 128;    //make sure tmp is [-127, 127]
    pss_ly = (unsigned char)tmp; 

    if(abs(pss_lx) > 50 || abs(pss_ly) > 50) {
      moveXY(pss_lx, pss_ly);
    };


// The right joystick is just the same as the left one.
    tmp = ps2x.Analog(PSS_RX);
    tmp = tmp == 0? -127 : tmp - 128;    //make sure tmp is [-127, 127]
    pss_lx = (unsigned char)tmp; 

    tmp = ps2x.Analog(PSS_RY);
    tmp = tmp == 0? -127 : tmp - 128;    //make sure tmp is [-127, 127]
    pss_ly = (unsigned char)tmp; 

    if(abs(pss_lx) > 50 || abs(pss_ly) > 50) {
      moveXY(pss_lx, pss_ly);
    };

    delay(50);

    if(need_stop) {
	need_stop = 0;

        for(int i=0; i<sizeof(stop); i++) {
           Serial.write(stop[i]);
        }
    }
}
