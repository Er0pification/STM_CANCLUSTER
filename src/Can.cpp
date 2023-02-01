#include "Can.h"
#include "fiat_msg.h"
#include "flags.h"

/*
##############
TRIP##0000.0KM
AVG##15.5L/100
INST#15.5L/100
INST##15.5L/HR
BATT#####14.5V
CLT######100oC
OILT#####100oC
BOOST####68KPA
*/



eXoCAN can;

/*struct msg {
    char message[15];
    uint8_t pos;
    bool isInt;
    int maxval;
    int *value;
};*/
uint8_t currentMsg, prevMsg;
int clt;
int oilt;
int avgF;
int instF;
int voltage;
uint16_t rpm;
uint8_t speed;
uint8_t fuel;
int trip_counter;
int outside_temp;
uint8_t fuel_ltr;
uint8_t fuel_capacity;
uint8_t fuel_low_level;
char msgBuff[15];
bool cluster_awake = false;
bool clr = false;
uint8_t odo_cnt;
uint8_t btn_prev;
uint8_t prev_msg;

bool halfspeed;


//struct msg message[MsgNum];



void canISR() // get bus msg frame passed by a filter to FIFO0
{
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes); // get CAN msg
  switch (can.id)
  {
  case ID_DIMMER:
    if (can.rxData.bytes[DIMM_SENSOR_BYTE] & DIMM_SENSOR ) {
      F_BCL = true;
    }
    else {
      F_BCL = false;
    }
    break;

  case ID_STATUS:
    if (can.rxData.bytes[STATUS_BYTE] & STATUS_AWAKE) cluster_awake = true;
    else cluster_awake = false;
    break;

  case ID_INDICATOR:
    if (can.rxData.bytes[0] & LOW_BRAKE_FLUID) F_BRAKE_FLUID = true; else F_BRAKE_FLUID = false;
    if (can.rxData.bytes[0] & CHECK_PADS) F_BRAKE_PADS = true; else F_BRAKE_PADS = false;
    if (can.rxData.bytes[0] & PARK_BRAKE) F_BRAKE_PARK = true; else F_BRAKE_PARK = false;

    if (can.rxData.bytes[1] & DOOR_RF) F_DOOR_RF = true; else F_DOOR_RF = false;
    if (can.rxData.bytes[1] & DOOR_LF) F_DOOR_LF = true; else F_DOOR_LF = false;
    if (can.rxData.bytes[1] & DOOR_IND) F_DOOR = true; else F_DOOR = false;
    if (can.rxData.bytes[1] & BOOT) F_DOOR_BOOT = true; else F_DOOR_BOOT = false;

    uint8_t btn_current = (can.rxData.bytes[2] & BTN_PRESS_MSK);
    uint8_t btn_inc;
    if (btn_current< btn_prev) btn_inc = btn_current+ BTN_PRESS_MSK - btn_prev; //if overflow occured
    else btn_inc = btn_current - btn_prev;
    if (can.rxData.bytes[2] & BTN_LONGPRESS){ tripReset();}
    while (btn_inc) 
    {
      NextMsg();
      btn_inc --;
    }
    btn_prev = btn_current;
    if (can.rxData.bytes[6] & LOCK_IND_BLINK) F_LOCK_BLINK = true; else F_LOCK_BLINK = false;
    if (can.rxData.bytes[6] & LOCK_IND) F_LOCK = true; else F_LOCK = false;
  break;

  }
}

void InitializeCan (){
    can.begin(EXT_ID_LEN, BR50K, PORTB_8_9_XCVR); // 29b IDs, 250k bit rate, transceiver chip, portB pins 8,9
  //can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
    can.attachInterrupt(canISR);

   currentMsg = msg_tripA;

}

void SweepIndicators (void)
{
  ClrText();
  speed = 0;
  rpm = 0;
  ClusterFramesSend();  
  speed = MAX_SPEED;
  rpm = MAX_RPM;
  ClusterFramesSend();  
  delay(sweep_time);
  speed = 0;
  rpm = 0;
  ClusterFramesSend();  
}

void ClusterFramesSend (void){
  unsigned char Data[8];
  unsigned char Byte;
  halfspeed = !halfspeed;
  /*//DRAFT
  Data[0] = 0;
  Data[1] = 0;
  Data[2] = 0;
  Data[3] = 0;
  Data[4] = 0;
  Data[5] = 0;
  Data[6] = 0;
  Data[7] = 0;
  can.transmit(DRAFT, Data, 8);*/


  //0x221400 //headlight and stuff
  /*should be handled at second node*/
  /*Data[0] = 0;

  Byte = 0;
  if (F_FOG_REAR)     Byte+=FOG_REAR;
  if (F_FOG_FRONT)    Byte+=FOG_FRONT;
  if (F_HEADLIGHT_CORRECTOR)      Byte+=LAMP_CORR;
  if (F_HIBEAM)     Byte+=HIBEAM;
  if (F_SIDELAMP)     Byte+=SIDELAMP;
  Data[1] = Byte;

  Byte = 0;
  if (F_LTURN)     Byte+=LTURN;
  if (F_RTURN)     Byte+=RTURN;
  Data[2] = Byte;

  Data[3] = 0;
  Data[4] = 0;
  Data[5] = 0;
  Data[6] = 0;
  Data[7] = 0;
  CanSend(0x221400, Data, 8);*/

  //0x04214001 //RPM CLT and engine status
  Data[0] = 0;
  Byte = 0;
  if (F_GLOW)     Byte+=GLOW_IND;
  if (F_GLOW_ERR)     Byte+=GLOW_CHECK;
  if (F_FILTER_ERR)     Byte+=FUEL_FILTER_IND;
  if (F_GENERAL_WARNING)     Byte+=WARNING_IND;
  if (F_OILPRESS)     Byte+=OILPRESS_IND;
  Data[1] = Byte;

  Byte = 0;
  if (F_OVERHEAT)     Byte+=OVERHEAT_IND;
  if (F_FUEL_CUTOFF)     Byte+=F_CUTOFF;
  if (F_CHECK_ENGINE)     Byte+=CHECKENGINE_IND;
  if (F_CHECK_ENGINE_BLINK)     Byte+=CHECKENGINE_IND_BLINK;
  Data[2] = Byte;

  F_OVERHEAT = false;
  if (clt<60) Byte = 0x50;
  else if (clt<70) Byte = 0x5a;
  else if (clt<80) Byte = 0x64;
  else if (clt<90) Byte = 0x70;
  else if (clt<95) Byte = 0x9c;
  else if (clt<100) Byte = 0x9f;
  else if (clt<110) Byte = 0xa2;
  else {
    Byte = 0xa0; F_OVERHEAT = true;
  }
  Data[3] = Byte; //CLT

  Data[4] = 0;
  Data[5] = 0;

  if (rpm>8000) rpm = 8000;
  Byte = rpm/32;
  Data[6] = Byte; //RPM

  Byte = 0;
  if (F_OIL_BLINK) Byte+=OIL_BLINK;
  if (F_OIL_CHANGE) Byte+=OIL_CHANGE;
  if (F_OIL_LEVEL) Byte+=OIL_LEVEL_MSG;
  Data[7] = Byte;
  CanSend(0x04214001, Data, 8);  

  //0x04214002 EPAS
  Data[0] = 0;
  if (F_EPAS_ERR) Byte+=EPAS_FAIL;
  Data[1] = Byte;
  CanSend(0x04214002, Data, 2);

  //0x04214006 ABS indicator
  Data[0] = 0;

  Byte = 0;
  if (F_ABS) Byte+=ABS_UNAVAIL;
  if (F_EBD_ERR) Byte+=EBD_FAIL;
  Data[1] = Byte;
  Data[2] = 0;
  Data[3] = 0;
  Data[4] = 0;
  Data[5] = 0;
  Data[6] = 0;
  Data[7] = 0;
  CanSend(0x04214006, Data, 8); 

  //0x04394000 Speedometer
  uint16_t speed_tmp = speed*14.9;
  Data[0] = hi8(speed_tmp);
  Data[1] = lo8(speed_tmp);
  Data[2] = 0;
  Data[3] = odo_cnt;
  CanSend(0x04394000, Data, 4);


  //0x06214000 FUEL
  Byte = 0;
  if (F_BCL) Byte+=BCL_EN;
  if (F_BRAKE_FLUID) Byte+=LOW_BRAKE_FLUID;
  if (F_BRAKE_PARK) Byte+=PARK_BRAKE;
  if (F_BRAKE_PADS) Byte+=CHECK_PADS;
  Data[0] = Byte;

  Byte = 0;
  if (F_DOOR) Byte+=DOOR_IND;
  if (F_DOOR_RF) Byte+=DOOR_RF;
  if (F_DOOR_LF) Byte+=DOOR_LF;
  if (F_DOOR_BOOT) Byte+=BOOT;
  Data[1] = Byte;

  Byte = 0;
  if (F_ENABLE) Byte+=CLUSTER_ENABLE;
  Data[2] = Byte;

  Data[3] = 0;

  Byte = 0;
  if (F_FUEL_LOW) Byte+=LOW_FUEL_IND;
  if (F_FUEL_VERYLOW) Byte+=LOW_FUEL_BLINK;
  Data[4] = Byte;

  Data[5] = fuel;

  Byte = 0;
  if (F_LOCK) Byte+=LOCK_IND;
  if (F_LOCK_BLINK) Byte+=LOCK_IND_BLINK;
  Data[6] = Byte;
  Data[7] = 0;
  CanSend(0x06214000, Data, 8);

  
  if (halfspeed) //theese can be transmitted at half frequency
  {
    //0x0621401A Belt stuff. Die like a real man
      Byte = 0;
      if (F_AIRBAG_BLINK) Byte+=AIRBAG_BLINK;
      if (F_AIRBAG) Byte+=AIRBAG_FAIL;
      if (F_PASS_AIRBAG_BLINK) Byte+=PASS_AIRBAG_BLINK;
      if (F_PASS_AIRBAG) Byte+=PASS_AIRBAG;
      Data[0] = Byte;

      Byte = 0;
      if (F_BELT) Byte+= BELT;
      Data[1] = Byte;
      Byte = 0;
      if (F_BELT) Byte+= BELT;
      Data[2] = Byte;
      Data[3] = 0;
      Data[4] = 0;
      Data[5] = 0;
      Data[6] = 0;
      Data[7] = 0;
      CanSend(0x0621401A, Data, 8);
      
      //0x06314000 Charge
      Byte = 0;
      if (F_CHARGE) Byte+=CHARGE;
      if (F_LOWBATT) Byte+= CHARGE_BLINK;
      Data[0] = Byte;
      Data[1] = 0;
      Data[2] = 0;
      Data[3] = 0;
      Data[4] = 0;
      Byte = 0;
      if (F_CITY) Byte+=MODE_CITY;
      if (F_SPORT) Byte+=MODE_SPORT;
      Data[5] = Byte;
      Data[6] = 0;
      Data[7] = 0;
      CanSend(0x06314000, Data, 8);  

      //0x063D4000 outside temp (T+40)*2 FROM -39 t0 88
      if (outside_temp>88) outside_temp = 88;
      Byte = (outside_temp+40)*2;
      Data[0] = Byte;
      Data[1] = 0;
      Data[2] = 0;
      Data[3] = 0;
      CanSend(0x063D4000, Data, 4);
  }
}


void CanSend(long MsgID, unsigned char *Data, char msgLen) {
  while (!can.txReady());
  can.transmit(MsgID, Data, msgLen);
}


void CanText (char *message){
  unsigned char Data[8];
  int inputLen = strlen(message);
  
  int encodedLen = base64_enc_len(message);
  char encoded[encodedLen];
  base64_encode(message, encoded);
  /*
    * 4 bits for a message length, starting from 0 (1=two messages)
    * 4 bits for the message number, starting from 0
    * 4 bits for the destination display (1=dashboard, 2=radio unit)
    * 4 bits constantly 0xA*/
  Data[0] = 0x10;
  Data[1] = 0x10 + MSG_ARROW_NONE;
  int current = 0;
  for (uint8_t i = 2; i <=7; i++)
  {
    Data[i] = encoded[current++];
  }
  CanSend(0x0a394021, Data, 8);
  Data[0] = 0x11;
  Data[1] = 0x10 + MSG_ARROW_NONE;
  for (uint8_t i = 2; i <=7; i++)
  {
    if (current <= encodedLen)
    Data[i] = encoded[current++];
    else 
    Data[i] = 0;
  }
  delay(15);
  CanSend(0x0a394021, Data, 8);
   
}

void ClrText (void)
{
    unsigned char Data[8];
    Data[0] = 0;
    Data[1] = 0;
    CanSend(0x0a394021, Data, 8);
    delay(15);
    CanSend(0x0a394021, Data, 8);
}

void UpdateText (void){
    prev_msg = data.currentMsg;
    if (no_resp) data.currentMsg = msg_Timeout;
    else if (!data_valid) data.currentMsg = msg_Corrupt;
    else if (eeprom_corrupt) data.currentMsg = msg_EECorrupt;
    else data.currentMsg = prev_msg;
    if (data.currentMsg == Def)
    {
      if (!clr)
      {
        ClrText();
        clr = true;
      }
      
    } 
    else {
      ValueToText();
      CanText(msgBuff);      
    }
  
}

void ValueToText (void){

  switch(data.currentMsg){
    //info messages
    case msg_tripA:
      trip_counter = (int)(data.tripMeter/100);
      sprintf(msgBuff,"TRIP%6d.%dKM", trip_counter/10, trip_counter%10); //sprinf with floats uses shit ton of resources
      break;
    case msg_tripL:
      trip_counter = (int)(data.CumulativeFuel/100);
      sprintf(msgBuff,"CONSUMED%3d.%dL", trip_counter/10, trip_counter%10); //sprinf with floats uses shit ton of resources
      break;
    case msg_AvgCons:
      if (avgF>500) sprintf(msgBuff,"AVG  --.-L/100", avgF/10, avgF%10);
      else sprintf(msgBuff,"AVG%4d.%dL/100", avgF/10, avgF%10);
      break;
    case msg_InstCons:
      if (speed<=5)
      sprintf(msgBuff,"INST%3d.%dL/HR ", instF/10, instF%10);
      else
      {
        instF = instF * 100 / speed;
        sprintf(msgBuff,"INST%3d.%dL/100", instF/10, instF%10);
      }      
      break;
    case msg_BattV:
      sprintf(msgBuff,"BATT%7d.%dV", voltage/10, avgF%10);
      break;
    case msg_CltT:
      //sprintf(msgBuff,"CLT%9doC", clt);
      sprintf(msgBuff,"COOLANT%5doC", clt);
      break;
    case msg_OilT:
      sprintf(msgBuff,"OIL T%7doC", oilt);
      break;
    //errors msg_Timeout, msg_Corrupt
    case msg_Timeout:
      sprintf(msgBuff," COMM TIMEOUT!");
      break;
    case msg_Corrupt:
      sprintf(msgBuff,"  COMM ERROR! ");
      break;
    case msg_EECorrupt:
      sprintf(msgBuff," EEPROM FAIL!");
      break;
    default:
      sprintf(msgBuff,"    WTF?!    ");
      break;

    
  }

}

void NextMsg (void){
  //prevMsg = currentMsg;
  data.currentMsg ++;
  if (data.currentMsg>LAST_MSG) 
  {
    data.currentMsg = 0;
    clr = false;
  }
}