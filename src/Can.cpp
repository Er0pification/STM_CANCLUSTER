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

struct msg {
    char message[15];
    uint8_t pos;
    bool isInt;
    int maxval;
    int *value;
};
uint8_t currentMsg, prevMsg;
int clt;
int oilt;
int avgF = 145;
int instF = 276;
int voltage;
uint16_t rpm;
uint8_t speed;
uint8_t fuel;
int trip_counter;
int outside_temp;
uint8_t fuel_ltr;
uint8_t fuel_capacity;
uint8_t fuel_low_level;
int prevVal;
char msgBuff[15];


struct msg message[MsgNum];



void canISR() // get bus msg frame passed by a filter to FIFO0
{
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes); // get CAN msg
  if (can.id == DIMMER_ID){
    if (can.rxData.bytes[DIMM_SENSOR_BYTE] && DIMM_SENSOR ) {
      F_BCL = true;
    }
    else {
      F_BCL = false;
      NextMsg();
    }
  }
 
}

void InitializeCan (){
    can.begin(EXT_ID_LEN, BR50K, PORTB_8_9_XCVR); // 29b IDs, 250k bit rate, transceiver chip, portB pins 8,9
  //can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
    can.attachInterrupt(canISR);

    
    
    strcpy(message[tripA].message, "TRIP        KM");
    message[tripA].pos = 11;
    message[tripA].isInt = false;
    message[tripA].maxval = 99999;
    message[tripA].value = &trip_counter;

    strcpy(message[AvgCons].message, "AVG      L/100");
    message[AvgCons].pos = 8;
    message[AvgCons].isInt = false;
    message[AvgCons].maxval = 999;
    message[AvgCons].value = &avgF;

    strcpy(message[InstCons].message, "INST     L/100");
    message[InstCons].pos = 8;
    message[InstCons].isInt = false;
    message[InstCons].maxval = 999;
    message[InstCons].value = &instF;
    
    strcpy(message[BattV].message, "BATT         V");
    message[BattV].pos = 12;
    message[BattV].isInt = false;
    message[BattV].maxval = 200;
    message[BattV].value = &voltage;

    strcpy(message[CltT].message, "CLT         oC");
    message[CltT].pos = 11;
    message[CltT].isInt = true;
    message[CltT].maxval = 140;
    message[CltT].value = &clt;

    strcpy(message[OilT].message, "OILT        oC");
    message[OilT].pos = 11;
    message[OilT].isInt = true;
    message[OilT].maxval = 160;
    message[OilT].value = &oilt;

    currentMsg = tripA;

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
  Data[0] = 0;

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
  CanSend(0x221400, Data, 8);

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
  Data[3] = trip_counter;
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
  Data[5] = 0;
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
  delay(5);
  CanSend(0x0a394021, Data, 8);
   
}

void ClrText (void)
{
    unsigned char Data[8];
    Data[0] = 0;
    Data[1] = 0;
    CanSend(0x0a394021, Data, 8);
    delay(5);
    CanSend(0x0a394021, Data, 8);
}

void UpdateText (void){
  int val = *message[currentMsg].value;
  if (currentMsg != prevMsg || val!=prevVal){//update text only if message or value has changed
    if (currentMsg == Def) ClrText();
    else {
      strcpy(msgBuff, message[currentMsg].message);
      ValueToText();
      CanText(msgBuff);      
    }
    prevMsg = currentMsg;
    prevVal = val;
  }
  
}

char ValueToText (void){
  int val = *message[currentMsg].value;
  int valLen = 0;
  int tmp = val;
  bool negative = false;
  if (val<0) {
    negative = true;
    val*=(-1);
  }
  while (tmp != 0) {
    tmp /= 10;
    ++valLen;
    }
  tmp = message[currentMsg].pos; 
  msgBuff[tmp--] = (val%10)+'0';
  if (!message[currentMsg].isInt) {
    msgBuff[tmp--] = '.';
  }
  else if (valLen == 1) return 0;
  while (valLen>1){
      val /= 10;
      valLen--;
      msgBuff[tmp--] = (val%10) +'0';
  }
  if (negative) msgBuff[tmp--] = '-';
  return 0;
}

void NextMsg (void){
  //prevMsg = currentMsg;
  currentMsg ++;
  if (currentMsg>=MsgNum) currentMsg = 0;
}