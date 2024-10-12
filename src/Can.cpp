#include "Can.h"
#include "flags.h"
#include "rusefi_broadcast.h"
eXoCAN can;

uint8_t currentGear;
uint16_t trip;
uint16_t rpm;
float ignTiming;
uint8_t injDuty;
uint8_t vehicleSpeed;
uint8_t wastegatePos;
uint8_t tps;
uint8_t pps;
float manifoldPress;
int16_t clt;
int16_t iat;
int16_t aux1t;
int16_t aux2t;
int16_t mcut;
uint8_t fuelLevel;
float oilPres;
int16_t oilt;
int16_t fuelt;
float batt;
float fuelFlow;
float fuelUsed;
float lambda1;
float lambda2;
float fuelPres;
uint32_t msg_cnt;

void canISR() // get bus msg frame passed by a filter to FIFO0
{
  byte byte;
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes); // get CAN msg
  switch (can.id)
  {
    case BASE_ID:
      byte = can.rxData.bytes[4];
      if (byte & 1<<REV_LIM_OFF ) {
        F_REV_LIMITER = true;
      }
      else {
        F_REV_LIMITER = false;
      }
      if (byte & 1<<MAIN_RLY_OFF ) {
        F_MAIN_RELAY = true;
      }
      else {
        F_MAIN_RELAY = false;
      }
      if (byte & 1<<FUEL_PUMP_OFF ) {
        F_FUEL_PUMP = true;
      }
      else {
        F_FUEL_PUMP = false;
      }
      if (byte & 1<<CHECK_OFF ) {
        F_CHECK_ENGINE = true;
      }
      else {
        F_CHECK_ENGINE = false;
      }
      if (byte & 1<<FAN1_OFF ) {
        F_FAN1 = true;
      }
      else {
        F_FAN1= false;
      }
      if (byte & 1<<FAN2_OFF ) {
        F_FAN2 = true;
      }
      else {
        F_FAN2 = false;
      }
      currentGear = can.rxData.bytes[5];
      trip = ((can.rxData.bytes[7]<<8)&(can.rxData.bytes[8]))/10;
      msg_cnt++;
      break;
    case BASE_ID+1:
        rpm = ((can.rxData.bytes[0]<<8)&(can.rxData.bytes[1]));
        ignTiming = ((can.rxData.bytes[2]<<8)&(can.rxData.bytes[3]))/50;
        injDuty = can.rxData.bytes[4]/2;
        vehicleSpeed = can.rxData.bytes[6];
      break;
    case BASE_ID+2:
        pps = ((can.rxData.bytes[0]<<8)&(can.rxData.bytes[1]))/100;
        tps = ((can.rxData.bytes[2]<<8)&(can.rxData.bytes[3]))/100;
        wastegatePos = ((can.rxData.bytes[6]<<8)&(can.rxData.bytes[7]))/100;
      break;
    case BASE_ID+3:
        manifoldPress = ((can.rxData.bytes[0]<<8)&(can.rxData.bytes[1]))/30;
        clt = can.rxData.bytes[2]-40;
        iat = can.rxData.bytes[3]-40;
        aux1t = can.rxData.bytes[4]-40;
        aux2t = can.rxData.bytes[5]-40;
        mcut = can.rxData.bytes[6]-40;
        fuelLevel = can.rxData.bytes[7]/2;
      break;
    case BASE_ID+4:
      oilPres = ((can.rxData.bytes[2]<<8)&(can.rxData.bytes[3]))/30;
      oilt = can.rxData.bytes[4]-40;
      fuelt = can.rxData.bytes[5]-40;
      batt = ((can.rxData.bytes[6]<<8)&(can.rxData.bytes[7]))/1000;
      break;
    case BASE_ID+6:
      fuelUsed = ((can.rxData.bytes[0]<<8)&(can.rxData.bytes[1]));
      fuelFlow = ((can.rxData.bytes[2]<<8)&(can.rxData.bytes[3]))/2000;
      break;
    case BASE_ID+7:
      lambda1 = ((can.rxData.bytes[0]<<8)&(can.rxData.bytes[1]))/10000;
      lambda2 = ((can.rxData.bytes[2]<<8)&(can.rxData.bytes[3]))/10000;
      fuelPres = ((can.rxData.bytes[4]<<8)&(can.rxData.bytes[5]))/30;
      break;

  }
}

void InitializeCan (){
  lcd_terminal("Initialize CAN-bus...",0);
    can.begin(EXT_ID_LEN, BR500K, PORTA_11_12_XCVR); // 29b IDs, 250k bit rate, transceiver chip, portB pins 8,9
  //can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
    can.attachInterrupt(canISR);
    lcd_terminal("Succesful!",0);
}





void CanSend(long MsgID, unsigned char *Data, char msgLen) {
 // while (!can.txReady());
  can.transmit(MsgID, Data, msgLen);
}




