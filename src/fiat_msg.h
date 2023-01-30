//ID
//byte 0
//byte 1
//byte 2
//byte 3
//byte 4
//byte 5
//byte 6
//byte 7

//0x04214002
//dlc 2
//byte 0
//byte 1
#define EPAS_FAIL 0x80


//0x06214000
/*Brake Fluid, Park brake, Doors, Locks - handled at secondary node*/
//byte 0
#define BCL_EN 0x08
#define LOW_BRAKE_FLUID 0x40
#define CHECK_PADS 0x80
#define PARK_BRAKE 0x20
//byte 1
#define DOOR_RF 0x04
#define DOOR_LF 0x08
#define BOOT 0x40
#define DOOR_IND 0x80
//byte 2
#define CLUSTER_ENABLE 0x40
//byte 3
//byte 4
#define LOW_FUEL_BLINK 0x01
#define LOW_FUEL_IND 0x02
//byte 5
// 0x01 to 0x60 - fuel level
//byte 6
#define LOCK_IND_BLINK 0x01
#define LOCK_IND 0x02 //0x04
#define F_CUTOFF_UA 0x80
//byte 7

//0x06214007 secondary node message
#define ID_INDICATOR 0x06214007
/*Brake Fluid, Park brake, Doors, Locks - handled at secondary node
//byte 0
#define LOW_BRAKE_FLUID 0x40
#define CHECK_PADS 0x80
#define PARK_BRAKE 0x20
//byte 1
#define DOOR_RF 0x04
#define DOOR_LF 0x08
#define BOOT 0x40
#define DOOR_IND 0x80*/
//byte 2
#define BTN_LONGPRESS 0x80
#define BTN_PRESS_MSK 0x7F
//byte 3
//byte 4
//byte 5
//byte 6
/*#define LOCK_IND_BLINK 0x01
#define LOCK_IND 0x02 //0x04
//byte 7*/

//ID 04214001 
/*can be handled fully at main node*/
//byte 0
//byte 1
#define GLOW_IND 0x10
#define FUEL_FILTER_IND 0x20 //check diesel fuel filter message
#define WARNING_IND 0x40 // warning indicator without message
#define OILPRESS_IND 0x80
#define GLOW_CHECK 0xff //check glow plugs message
//byte 2
#define OVERHEAT_IND 0x01
#define F_CUTOFF 0x02
#define CHECKENGINE_IND_BLINK 0x08
#define CHECKENGINE_IND 0x10
//byte 3
//CLT 
//1
//0x01-0x50
//2 0x60
//3 0x65
//4 0x70 //middle point
//5 0x9C
//6 0x9F
//7 0xA0
//byte 4
//byte 5
//byte 6
//tachometer RPM/32
//byte 7
#define OIL_BLINK 0x01
#define OIL_CHANGE 0x02
#define OIL_LEVEL_MSG 0x10

//ID 04214006
/*can be handled fully at main node*/
//byte 0
//byte 1
#define EBD_FAIL 0x10
#define ABS_UNAVAIL 0x20
//byte 2
#define ASR_DISCONN 0x01
//byte 3
//byte 4
//byte 5
//byte 6
//byte 7

//ID 04394000
/*can be handled fully at main node*/
//byte 0 - 1
//speed in km/h*14.9 or in miles/hr *24
//byte 2
// allways 0?
//byte 3
//trip counter
//0x42 for each km
#define KM_STEPS 0x42

//ID 02214000
/*should be handled at second node*/
//byte 0
//byte 1
#define FOG_REAR 0x02
#define FOG_FRONT 0x04
#define LAMP_CORR 0x08
#define HIBEAM 0x10
#define SIDELAMP 0x20

//byte 2
#define LTURN_CHECK 0x01
#define RTURN_CHECK 0x01
#define RTURN 0x20
#define LTURN 0x40
//byte 3
#define CHECK_RFOG 0x01
#define CHECK_RFOG_IND 0x02
#define CHECK_STOP 0x08
#define CHECK_STOP_IND 0x10

//byte 4
//byte 5// do not care....


//ID 063D4000 outside temp
/*half speed*/
//byte 0
/*T, CELSIUS
(T+40)*2 FROM -39 t0 88
*/
//byte 1
//byte 2
//byte 3
//byte 4
//byte 5
//byte 6
//byte 7

//ID 0x06314000
/*half speed*/
//byte 0
// 0x02 responds with 0x1* at 0x06314003 byte 5
// 0x04 responds with 0x2* at 0x06314003 byte 5
// 0x06 responds with 0x3* at 0x06314003 byte 5
//somehow connected to dimmer, haven't figured it out yet 
#define CHARGE 0x40
#define CHARGE_BLINK 0x80
//byte 1
//byte 2
//byte 3
//byte 4
//byte 5
#define MODE_CITY 0x01
#define MODE_SPORT 0x02
//byte 6
//byte 7

//ID 0x06314003

#define ID_DIMMER 0x06314003
//byte 0
//byte 1
//byte 2 dimmer settings 0x00 to 0x70
#define DIMM_BYTE 2
//byte 3
//byte 4
//byte 5
#define DIMM_SENSOR_BYTE 5
#define DIMM_SENSOR 0x08 // in dark condition, NULL if bright
//byte 6
//byte 7


//ID 0x0c394003
//byte 0
#define UNITS_KM 0x10
#define UNITS_MI 0x92
#define UNITS_L100 0x01
//byte 1
//byte 2
//byte 3
#define AUTOCLOSE 0x10
//byte 4
//byte 5
#define DAYLIGHTS 0x20
//byte 6
//byte 7

//ID 0x0a394021 text message
//byte 0
    //4 bits for a message length, starting from 0 (1=two messages)
    //4 bits for the message number, starting from 0
//byte 1
    //4 bits for the destination display (1=dashboard, 2=radio unit)
    //4 bits constantly 0xA --  not really
#define MSG_ARROW_NONE 0x06
#define MSG_ARROW_RIGTH 0x0A
#define MSG_ARROW_BOTH 0x08
//byte 2
//byte 3
//byte 4
//byte 5
//byte 6
//byte 7

//ID 0x0621401a
/*half speed*/
//byte 0
#define PASS_AIRBAG_BLINK 0x10 
#define PASS_AIRBAG 0x20 
#define AIRBAG_BLINK 0x40
#define AIRBAG_FAIL 0x80
//byte 1
#define BELT 0x01
//byte 2
//byte 3
//byte 4
//byte 5
//byte 6
//byte 7

#define ID_STATUS 0x0E094003
//byte 0
//byte 1 
#define STATUS_AWAKE 0x1E
#define STATUS_ASLEEP 0x1C
#define STATUS_BYTE 1


#define MAX_SPEED 240
#define MAX_RPM 8000

