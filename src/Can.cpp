#include "Can.h"
#include "fiat_msg.h"
#include "flags.h"
#include "hardware.h"

/*
##############
TRIP##0000.0KM
AVG##15.5L/100
BATT#####14.5V
CLT######100oC
OILT#####100oC
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
int clt ;
int oilt;
int avgF;
//int instF, instF_prev;
float alfa = 0.1;
float filtered_fuel_level = 0;
//float PW;
//int16_t MAP;
uint16_t rpm ;
uint16_t speed;
uint8_t fuel;
int voltage;
int trip_counter;
int outside_temp;
char msgBuff[15];
bool cluster_awake = false;
bool clr = false;
bool first_run = true;
uint8_t frame_time;
uint32_t  time_prev;
float trip;
//uint8_t trip_inc, trip_inc_prev;
double odo_tmp;
uint8_t odo_cnt;
//double fuelCC;
uint16_t fuelConsumedG, fuelConsumedG_prev;


uint32_t time() //returns time interval to previous call in MS
{
  uint32_t result  = getCurrentMillis()- time_prev;
  time_prev = result;
  
  return result;
}

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

  /*case ID_STATUS:
    if (can.rxData.bytes[STATUS_BYTE] & STATUS_AWAKE) cluster_awake = true;
    else cluster_awake = false;
    break;*/

  case ID_ECU_PACKET1:
      fuelConsumedG = (can.rxData.bytes[0]<<8)|(can.rxData.bytes[1]);
      clt = can.rxData.bytes[2] - 40;
      oilt = can.rxData.bytes[3] - 40;
      voltage = can.rxData.bytes[4];
      if(fuelConsumedG_prev<fuelConsumedG)//non-overflow condition
      {
          data.CumulativeFuel+= (fuelConsumedG-fuelConsumedG_prev)/0.76; // convert grams to cc
          //fuelCalc();
          
      }
      fuelConsumedG_prev = fuelConsumedG;
      outside_temp  = can.rxData.bytes[5] - 40;
      speed = can.rxData.bytes[6];
      fuel = can.rxData.bytes[7];
  break;
  case ID_ECU_PACKET2:
      rpm = (can.rxData.bytes[0]<<8)|(can.rxData.bytes[1]);
      uint8_t WMI = can.rxData.bytes[2];
      if (WMI>0) 
      {
        F_GENERAL_WARNING = true;
        F_FILTER_ERR = true;
        if (WMI == 1)
        data.currentMsg =msg_WMI_LOW;
        else if (WMI == 2)
        data.currentMsg =msg_WMI_EMPTY;
        if (WMI == 3)
        {
          data.currentMsg =msg_WMI_FAIL;
          F_CHECK_ENGINE = true;
        }
        

      }
      else 
      {
        F_GENERAL_WARNING = false;
        F_FILTER_ERR = false;
        F_CHECK_ENGINE = false;
      }
      digitalWrite(PC13, !digitalRead(PC13));
  break;

  }
}

void InitializeCan (){
    can.begin(EXT_ID_LEN, BR50K, PORTA_11_12_XCVR); // 29b IDs, 50k bit rate, transceiver chip, portA pins 11,12
  //can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
    //can.attachInterrupt(canISR); //no interrupt before indicator sweep
    can.filterList32Init(0,ID_DIMMER,ID_ECU_PACKET3);
  can.filterList32Init(1,ID_ECU_PACKET1,ID_ECU_PACKET2);
   
}

void fuelCalc (void)
{
    
    /*uint8_t injN=6;
    uint8_t nSquirts = 1;
    uint8_t div = 2;
    uint16_t baseFlow = 296;
    uint16_t basePressure = 270;

    float deadtime = 1;

    float fuelPres = basePressure + (MAP-100);//kpa
    float flowCor = baseFlow * sqrt(fuelPres/basePressure); //cc/min 
    double squirtsPerEvent = (rpm*time()/60000) * (injN * nSquirts / div);
    //                        RPS       Ms to S             3 injections
    //                      {Revolutions per event}           per revolution
    uint32_t squirtsPerHr =    rpm * 60 * injN * nSquirts / div;
    //                          RPH
    
    double fuelPerSquirt = ((PW-deadtime)/60000) * flowCor;
    //                      (ms         to min) * cc/min = cc
    data.CumulativeFuel += squirtsPerEvent * fuelPerSquirt;
    double lhrCons = (squirtsPerHr * fuelPerSquirt) / 100000.0; //TODO  - find out where excessive 00 came from - MUST BE 1000!
    instF = (int)(lhrCons*10);
    instF = alfa*instF + (1-alfa)* instF_prev;
    instF_prev = instF;*/
    double avgTemp = (data.CumulativeFuel/1000) / (data.tripMeter/100000);
    //                (        cc    to Liters) / (     meters to 100km ) = ltr/100km
        avgF = int(avgTemp*10);
    //DbgSerial.printf("/nFuel total - %d, avg - %d, sqirts - %d", (int)data.CumulativeFuel, avgF, (int)squirtsPerEvent);
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
//getRAW(Data);
  //CanSend(0x001,Data,8);


  
  //0x04214001 //RPM CLT and engine status
  if (rpm>500 && F_OILPRESS)
  {
    F_OILPRESS = false; // override oil pressure indicator to disable annoing cluster error message
    F_OIL_BLINK = true;
  }
  if (F_OILPRESS == false)
  {
    F_OIL_BLINK = false;
  }
  Data[0] = 0;
  Byte = 0;
  //if (F_GLOW)     Byte+=GLOW_IND;
  //if (F_GLOW_ERR)     Byte+=GLOW_CHECK;
  if (F_FILTER_ERR)     Byte+=FUEL_FILTER_IND;
  if (F_GENERAL_WARNING)     Byte+=WARNING_IND;
  if (F_OILPRESS)     Byte+=OILPRESS_IND;
  Data[1] = Byte;

  Byte = 0;
  if (F_OVERHEAT)     Byte+=OVERHEAT_IND;
  //if (F_FUEL_CUTOFF)     Byte+=F_CUTOFF;
  if (F_CHECK_ENGINE)     Byte+=CHECKENGINE_IND;
  if (F_CHECK_ENGINE_BLINK)     Byte+=CHECKENGINE_IND_BLINK;
  Data[2] = Byte;

  F_OVERHEAT = false;
  if (clt<50) Byte = 0x50;
  else if (clt<60) Byte = 0x5a;
  else if (clt<70) Byte = 0x64;
  else if (clt<80) Byte = 0x70;
  else if (clt<90) Byte = 0x9c;
  else if (clt<100) Byte = 0x9f;
  else if (clt<110) Byte = 0xa2;
  else {
    Byte = 0xa0; F_OVERHEAT = true;
  }
  Data[3] = Byte; //CLT

  Data[4] = 0;
  Data[5] = 0;

  //if (rpm>8000) rpm = 8000;
  Byte = rpm/32;
  Data[6] = Byte; //RPM

  Byte = 0;
  if (F_OIL_BLINK) Byte+=OIL_BLINK;
  //if (F_OIL_CHANGE) Byte+=OIL_CHANGE;
  //if (F_OIL_LEVEL) Byte+=OIL_LEVEL_MSG;
  Data[7] = Byte;
  CanSend(0x04214001, Data, 8); 

  //0x04394000 Speedometer
  //tripCalculate();
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
  //if (F_BRAKE_PADS) Byte+=CHECK_PADS;
  Data[0] = Byte;

  Byte = 0;
  if (F_DOOR) Byte+=DOOR_IND;
  //if (F_DOOR_RF) Byte+=DOOR_RF;
  //if (F_DOOR_LF) Byte+=DOOR_LF;
  //if (F_DOOR_BOOT) Byte+=BOOT;
  Data[1] = Byte;

  Data[2] = CLUSTER_ENABLE;
  //Data[2] = 0;

  Data[3] = 0;

  if (first_run) filtered_fuel_level = fuel; // diasble Exponential moving average for first reading
  else filtered_fuel_level = alfa*fuel + (1-alfa)*filtered_fuel_level;
  Byte = 0;
  if (filtered_fuel_level <= 15) Byte+=LOW_FUEL_BLINK; // less than 9 liters or 15% - float stops floating at this point
  else if (filtered_fuel_level <= 20) Byte+=LOW_FUEL_IND; // less than 12 liters or 20%
  
  Data[4] = Byte;
  
  Data[5] = (int)filtered_fuel_level;

  //Byte = 0;
  //if (F_LOCK) Byte+=LOCK_IND;
  //if (F_LOCK_BLINK) Byte+=LOCK_IND_BLINK;
  Data[6] = 0;
  Data[7] = 0;
  CanSend(0x06214000, Data, 8);

    
      //0x06314000 Charge
      Byte = 0;
      if (rpm<500) 
      {
        if (voltage>110) Byte+=CHARGE;
        else Byte+= CHARGE_BLINK;
      }
      else
      {
        if (voltage<110) Byte+=CHARGE_BLINK;
        else if (voltage<130) Byte+=CHARGE;
      }
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
  
}
void ClusterFastFramesSend (void){
  unsigned char Data[8];
  unsigned char Byte;

  //0x221400 //headlight and stuff
  Data[0] = 0;

  Byte = 0;
  //if (F_FOG_REAR)     Byte+=FOG_REAR;
  //if (F_FOG_FRONT)    Byte+=FOG_FRONT;
    
  if (F_HIBEAM)     Byte+=HIBEAM;
  if (F_SIDELAMP)     { Byte+=SIDELAMP; Byte+=LAMP_CORR;}
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
  CanSend(0x02214000, Data, 8);

}
void ClusterSlowFramesSend (void){
  unsigned char Data[8];
  unsigned char Byte;
  
  //0x04214002 EPAS
  Data[0] = 0;
  if (F_EPAS_ERR) Byte+=EPAS_FAIL;
  Data[1] = Byte;
  CanSend(0x04214002, Data, 2);

  //0x04214006 ABS indicator
  Data[0] = 0;

  Byte = 0;
  if (F_ABS) Byte+=ABS_UNAVAIL;
  //if (F_EBD_ERR) Byte+=EBD_FAIL;
  Data[1] = Byte;
  Data[2] = 0;
  Data[3] = 0;
  Data[4] = 0;
  Data[5] = 0;
  Data[6] = 0;
  Data[7] = 0;
  CanSend(0x04214006, Data, 8);   

    //0x0621401A Belt stuff. Die like a real man
      //Byte = 0;
      //if (F_AIRBAG_BLINK) Byte+=AIRBAG_BLINK;
      //if (F_AIRBAG) Byte+=AIRBAG_FAIL;
      //if (F_PASS_AIRBAG_BLINK) Byte+=PASS_AIRBAG_BLINK;
      //if (F_PASS_AIRBAG) Byte+=PASS_AIRBAG;
      Data[0] = 0;

      //Byte = 0;
      //if (F_BELT) Byte+= BELT;
      Data[1] = 0;
      //Byte = 0;
      //if (F_BELT) Byte+= BELT;
      Data[2] = 0;
      Data[3] = 0;
      Data[4] = 0;
      Data[5] = 0;
      Data[6] = 0;
      Data[7] = 0;
      CanSend(0x0621401A, Data, 8);

      //0x063D4000 outside temp (T+40)*2 FROM -39 to 88 
      if (outside_temp>88) outside_temp = 88;
      Byte = (outside_temp+40)*2;
      Data[0] = Byte;
      Data[1] = 0;
      Data[2] = 0;
      Data[3] = 0;
      CanSend(0x063D4000, Data, 4);
      uint8_t data[8] = {00,0x1E,00,00,00,00,00,01};
      CanSend(0x0E094000, data,8);
    first_run = false;
}

void tripCalculate (uint16_t time)
{
  //static uint32_t time = millis()-time;
  
  float increment   =   (speed / 3600.0)*(time);
  //                    =   m/s       * (millisec)
	odo_tmp += increment;
  data.tripMeter += increment; 
	odo_cnt = odo_tmp/10;
	if (odo_tmp > 2550)
		odo_tmp = odo_tmp - 2550;
}
void tripReset (void)
{
    data.tripMeter = 0;
    data.CumulativeFuel = 0;
}
void CanSend(long MsgID, unsigned char *Data, char msgLen) {
  
  if(can.transmit(MsgID, Data, msgLen)==false) digitalWrite(PC13, !digitalRead(PC13));
  
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
    /*prev_msg = data.currentMsg;/*
    if (no_resp) data.currentMsg = msg_Timeout;
    else if (!data_valid) data.currentMsg = msg_Corrupt;
    else if (eeprom_corrupt) data.currentMsg = msg_EECorrupt;
    else data.currentMsg = prev_msg;*/
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
    case msg_tripAVG:
    //AVG##15.5L/100
      sprintf(msgBuff,"AVG%4d.%dL/100", avgF/10, avgF%10); //sprinf with floats uses shit ton of resources
      break;
      case msg_Range:
    //RANGE####100KM
      sprintf(msgBuff,"RANGE%7dKM", 0.6*filtered_fuel_level/avgF*10); // convert fuel in % to litters  / divide by avg consumtion per 100 * multiplied by 100
      break;
    case msg_BattV:
      sprintf(msgBuff,"BATT%7d.%dV", voltage/10, voltage%10);
      break;
    case msg_CltT:
      //sprintf(msgBuff,"CLT%9doC", clt);
      sprintf(msgBuff,"COOLANT%5doC", clt);
      break;
    case msg_OilT:
      sprintf(msgBuff,"OIL T%7doC", oilt);
      break;
    //errors msg_Timeout, msg_Corrupt
    case msg_WMI_LOW:
      sprintf(msgBuff,"WMI TANK LOW! ");
      break;
    case msg_WMI_EMPTY:
      sprintf(msgBuff,"WMI TANK EMPTY");
      break;
    case msg_WMI_FAIL:
      sprintf(msgBuff,"   WMI FAIL!  ");
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
    data.currentMsg = Def;
    clr = false;
  }
}

void SweepIndicators (void)
{
  uint8_t data[8] = {00,0x1c,00,00,00,00,00,01};
  CanSend(0x0E094000, data,8);
  speed = 0;
  rpm = 0;
  ClusterFramesSend();
  delay(50);
  speed = 230;
  rpm = 7500;
  ClusterFramesSend();
  delay(sweep_time);
  speed = 0;
    rpm = 0;
  ClusterFramesSend();
  delay(50);
  can.attachInterrupt(canISR);
}

