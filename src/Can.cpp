#include "Can.h"
#include "flags.h"

eXoCAN can;




void canISR() // get bus msg frame passed by a filter to FIFO0
{
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes); // get CAN msg
  /*switch (can.id)
  {
  case ID_DIMMER:
    if (can.rxData.bytes[DIMM_SENSOR_BYTE] & DIMM_SENSOR ) {
      F_BCL = true;
    }
    else {
      F_BCL = false;
    }
    break;

  }*/
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




