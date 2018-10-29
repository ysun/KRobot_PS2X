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

//#define Serial.print() 
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
unsigned char need_send = 0;
unsigned char is_pre_stop = 0;

unsigned char data[10] = {0};
#define MAX_CMD_LEN 10

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

/*
    ps2_data_list[MeJOYSTICK_LX] = buffer[2];
    ps2_data_list[MeJOYSTICK_LY] = buffer[4];
    ps2_data_list[MeJOYSTICK_RX] = buffer[6];
    ps2_data_list[MeJOYSTICK_RY] = buffer[8];
    ps2_data_list[MeJOYSTICK_R1] = (buffer[3] & 0x01) == 0x01 ? true : false;
    ps2_data_list[MeJOYSTICK_R2] = (buffer[3] & 0x02) == 0x02 ? true : false;
    ps2_data_list[MeJOYSTICK_L1] = (buffer[3] & 0x04) == 0x04 ? true : false; 
    ps2_data_list[MeJOYSTICK_L2] = (buffer[3] & 0x08) == 0x08 ? true : false;
    ps2_data_list[MeJOYSTICK_MODE] = (buffer[3] & 0x10) ==0x10 ? true : false;
    ps2_data_list[MeJOYSTICK_TRIANGLE] = (buffer[5] & 0x01) == 0x01 ? true : false;
    ps2_data_list[MeJOYSTICK_XSHAPED] = (buffer[5] & 0x02) == 0x02 ? true : false;
    ps2_data_list[MeJOYSTICK_SQUARE] = (buffer[5] & 0x04) == 0x04 ? true : false;
    ps2_data_list[MeJOYSTICK_ROUND] = (buffer[5] & 0x08) == 0x08 ? true : false;
    ps2_data_list[MeJOYSTICK_START] = (buffer[5] & 0x10) == 0x10 ? true : false;
    ps2_data_list[MeJOYSTICK_UP] = (buffer[7] & 0x01) == 0x01 ? true : false;
    ps2_data_list[MeJOYSTICK_DOWN] = (buffer[7] & 0x02) == 0x02 ? true : false ;
    ps2_data_list[MeJOYSTICK_LEFT] = (buffer[7] & 0x04) == 0x04 ? true : false ;
    ps2_data_list[MeJOYSTICK_RIGHT] = (buffer[7] & 0x08) == 0x08 ? true : false ;
    ps2_data_list[MeJOYSTICK_SELECT] = (buffer[7] & 0x10) == 0x10 ? true : false ;
    ps2_data_list[MeJOYSTICK_BUTTON_L] = (buffer[3] & 0x20) == 0x20 ? true : false ;
    ps2_data_list[MeJOYSTICK_BUTTON_R] = (buffer[7] & 0x20) == 0x20 ? true : false ;
*/

#define POS_DATA_PSS_LX 2
#define POS_DATA_PSS_LY 4
#define POS_DATA_PSS_RX 6
#define POS_DATA_PSS_RY 8

#define POS_DATA_PSB_L1 3
#define POS_DATA_PSB_L2 3
#define POS_DATA_PSB_R1 3
#define POS_DATA_PSB_R2 3

#define MASK_PSB_R1 0x01
#define MASK_PSB_R2 0x02
#define MASK_PSB_L1 0x04
#define MASK_PSB_L2 0x08
#define MASK_PSB_MODE 0x10

#define POS_DATA_PSB_TRIANGLE 5
#define POS_DATA_PSB_CROSS 5
#define POS_DATA_PSB_SQUARE 5
#define POS_DATA_PSB_CIRCLE 5
#define POS_DATA_PSB_START 5

#define MASK_PSB_TRIANGLE 0x01
#define MASK_PSB_CROSS 0x02
#define MASK_PSB_SQUARE 0x04
#define MASK_PSB_CIRCLE 0x08
#define MASK_PSB_START 0x10

#define POS_DATA_PSB_PAD_UP 7
#define POS_DATA_PSB_PAD_DOWN 7
#define POS_DATA_PSB_PAD_LEFT 7
#define POS_DATA_PSB_PAD_RIGHT 7
#define POS_DATA_PSB_SELECT 7

#define MASK_PSB_PAD_UP 0x01
#define MASK_PSB_PAD_DOWN 0x02
#define MASK_PSB_PAD_LEFT 0x04
#define MASK_PSB_PAD_RIGHT 0x08
#define MASK_PSB_SELECT 0x10

#define LISTEN_EVENT(EVENT_NAME) \
	if(ps2x.Button((EVENT_NAME))) \
		data[POS_DATA_##EVENT_NAME] = data[POS_DATA_##EVENT_NAME] | MASK_##EVENT_NAME; 

#define LISTEN_EVENT_ANALOG(EVENT_NAME) {\
	unsigned char tmp = ps2x.Analog(EVENT_NAME); \
	data[POS_DATA_##EVENT_NAME] = tmp;} 

void pack_cmd(unsigned char cmd[]) {
	cmd[0] = 0xff;
	cmd[1] = 0x55;

	cmd[9] = cmd[2]+cmd[3]+cmd[4]+cmd[5]+cmd[6]+cmd[7]+cmd[8];
}
void send_cmd(unsigned char cmd[]) {
	for(int i=0; i<10; i++) {
		Serial.write(cmd[i]);
	}
}

void check_send(unsigned char cmd[]) {
	if ( cmd[3] != 0 || cmd[5] != 0 || cmd[7] != 0 ||\
		!(is_pre_stop == 1 && check_stop(cmd) == 1) )
		need_send = 1;
}

unsigned char check_stop(unsigned char cmd[]) {
	if(abs(cmd[2]-0x80) <= 2 &&\
	   abs(cmd[4]-0x80) <= 2 &&\
	   abs(cmd[6]-0x80) <= 2 &&\
	   abs(cmd[8]-0x80) <= 2) {
		return 1;
	}
	return 0;
}

void loop() { //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed

/*
    Following is example for event listening
    if(ps2x.Button(PSB_PAD_UP)) {
      data[POS_DATA_PSB_UP] = data[POS_DATA_PSB_UP] | MASK_PSB_UP;
    }

    unsigned int tmp = ps2x.Analog(PSS_LX);
    tmp = ps2x.Analog(PSS_LY);
*/

    LISTEN_EVENT_ANALOG(PSS_LX);
    LISTEN_EVENT_ANALOG(PSS_LY);
    LISTEN_EVENT_ANALOG(PSS_RX);
    LISTEN_EVENT_ANALOG(PSS_RY);

    LISTEN_EVENT(PSB_L1);
    LISTEN_EVENT(PSB_L2);
    LISTEN_EVENT(PSB_R1);
    LISTEN_EVENT(PSB_R2);

    LISTEN_EVENT(PSB_TRIANGLE);
    LISTEN_EVENT(PSB_CROSS);
    LISTEN_EVENT(PSB_SQUARE);
    LISTEN_EVENT(PSB_CIRCLE);
    LISTEN_EVENT(PSB_START);

    LISTEN_EVENT(PSB_PAD_UP);
    LISTEN_EVENT(PSB_PAD_DOWN);
    LISTEN_EVENT(PSB_PAD_LEFT);
    LISTEN_EVENT(PSB_PAD_RIGHT);
    LISTEN_EVENT(PSB_SELECT);

    check_send(data);

    if(need_send) { 
        pack_cmd(data);
        send_cmd(data);
	need_send = 0;
	is_pre_stop = check_stop(data);

	for(int i = 0; i < MAX_CMD_LEN; i++)
		data[i] = 0;
    }
    delay(50);
}
