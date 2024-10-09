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
float instF, instF_prev;
float deltaFuel = 0;
float deltaTrip = 0;
float instAlfa = 0.3;
float alfa = 0.1;
float filtered_fuel_level;
//float PW;
//int16_t MAP;
uint16_t rpm ;
uint16_t speed;
uint8_t fuel = 50;
uint8_t fuel_cnt = 0;
int voltage;
int trip_counter;
int outside_temp;
char msgBuff[15];
bool cluster_awake = false;
bool clr = false;
bool first_run = true;
uint8_t frame_time;
uint32_t  time_prev;
uint32_t dimmer_time;
uint32_t deltaT,dbgVal;
float trip;
bool rx_tick;
bool newdata;
//uint8_t trip_inc, trip_inc_prev;
double odo_tmp;
uint8_t odo_cnt;
uint8_t pendingMsg, msgTick;
bool showErr; 
#define ERROR_TICKS  10
//double fuelCC;
uint16_t fuelConsumedG;
uint16_t fuelConsumedG_prev = 0;

uint8_t wakeup[6] = {0x00, 0x1C, 0x00, 0x00, 0x00, 0x01};
uint8_t status1[6] = {0x00, 0x1E, 0x00, 0x00, 0x00, 0x01};
uint8_t status2[6] = {0x00, 0x1E, 0x00, 0x00, 0x00, 0x6f};
uint8_t dbg[2];

uint32_t time() //returns time interval to previous call in MS
{
  uint32_t t = millis();
  uint32_t result  = t- time_prev;
  time_prev = t;
  if (result>50 && result<=150) result = 100;
  else if (result>150 && result<250) result = 200;
  return result;
}

void canISR() // get bus msg frame passed by a filter to FIFO0
{
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes); // get CAN msg
  
  switch (can.id)
  {
  case ID_DIMMER:
    {
    if (can.rxData.bytes[DIMM_SENSOR_BYTE] & DIMM_SENSOR ) {
      F_BCL = true;
    }
    else {
      F_BCL = false;
    }
    dimmer_time = getCurrentMillis();
    }
    break;

  /*case ID_STATUS:
    if (can.rxData.bytes[STATUS_BYTE] & STATUS_AWAKE) cluster_awake = true;
    else cluster_awake = false;
    CanSend(ID_STATUS+1,dbg,2);
    break;*/

  case ID_ECU_PACKET1:
      fuelConsumedG = (can.rxData.bytes[0]<<8)|(can.rxData.bytes[1]);
      clt = can.rxData.bytes[2] - 40;
      oilt = can.rxData.bytes[3] - 40;
      voltage = can.rxData.bytes[4];
      if (first_run) fuelConsumedG_prev = fuelConsumedG;
      if(fuelConsumedG_prev<fuelConsumedG)//non-overflow condition
      {
          deltaFuel = fuelConsumedG-fuelConsumedG_prev;
          data.CumulativeFuel+= deltaFuel; // do not convert grams to cc
          //fuelCalc();
          
      }
      fuelConsumedG_prev = fuelConsumedG;
      outside_temp  = can.rxData.bytes[5] - 40;
      speed = can.rxData.bytes[6];
      //fuel = can.rxData.bytes[7];
  break;
  case ID_ECU_PACKET2:
      rpm = (can.rxData.bytes[0]<<8)|(can.rxData.bytes[1]);
      uint8_t WMI = can.rxData.bytes[2];
      if (WMI>0) 
      {
        
        F_FILTER_ERR = true;
        if (WMI == 1)
        pendingMsg =msg_WMI_LOW;
        else if (WMI == 2)
        pendingMsg =msg_WMI_EMPTY;
        F_GENERAL_WARNING = true;        
        if (WMI == 3)
        {
          pendingMsg =msg_WMI_FAIL;
          F_CHECK_ENGINE = true;
        }
        

      }
      else 
      {
        pendingMsg = 0;
        F_GENERAL_WARNING = false;
        F_FILTER_ERR = false;
        F_CHECK_ENGINE = false;
      }
      fuel = can.rxData.bytes[4];
      if (can.rxData.bytes[3]>0) F_SPORT = true;
      else F_SPORT = false;
      newdata = true;
      deltaT = time();
      dbgVal = deltaT;
      CanSend(ID_ECU_PACKET3,dbg,2);
      tripCalculate(deltaT);
      fuelCalc();
      ClusterFramesSend();
      if(rx_tick) ClusterSlowFramesSend();
      rx_tick =! rx_tick;
      
      
      //digitalWrite(PC13, !digitalRead(PC13));
  break;

  }
}

bool NoValidData ()
{
    if (deltaT>750)
    {
      rpm = 0;
      speed = 0;
      fuel = 50;
      ClusterFramesSend();
      ClusterSlowFramesSend();
      return true;
    }
    else return false;
}


void InitializeCan (){
    can.begin(EXT_ID_LEN, BR50K, PORTA_11_12_XCVR); // 29b IDs, 50k bit rate, transceiver chip, portA pins 11,12
  //can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
    //can.attachInterrupt(canISR); //no interrupt before indicator sweep
    can.filterList32Init(0,ID_ECU_PACKET1,ID_DIMMER);
  can.filterList32Init(1,ID_ECU_PACKET2,ID_ECU_PACKET3);
  //can.filterMask32Init(0,0xFFFFFFF, 0xFFFFFFFF);
   
}

void fuelCalc (void)
{
    double avgTemp = ((data.CumulativeFuel/0.71)/1000) / (data.tripMeter/100000);
    //                (        cc    to Liters) / (     meters to 100km ) = ltr/100km
    if (data.tripMeter <100) avgF = 0;
    else if (avgTemp>500) avgF=500;
     else   avgF = int(avgTemp*10);
    
    float InstTemp = ((instF/0.71)/1000); //convert g/s to l/s
    InstTemp  *= 3600; // convert l/s to l/hr
    if (speed>0) {
      InstTemp *= (100.0/float (speed));
    }
    
    instF = instAlfa * InstTemp + (1-instAlfa) * instF_prev;
    instF_prev = instF;
    

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
  if (rpm>200 && F_OILPRESS)
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
  /*(if (clt<40) Byte = 0x50;
  else if (clt<50) Byte = 0x5a;
  else if (clt<70) Byte = 0x64;
  else if (clt<80) Byte = 0x70;
  else if (clt<90) Byte = 0x9c;
  else if (clt<95) Byte = 0x9f;
  else if (clt<100) Byte = 0xa2;
  else {
    Byte = 0xa0; F_OVERHEAT = true;
  }*/
  if (clt>105) {
    Byte = 0xa0; F_OVERHEAT = true;
  }
  else if (clt>100) Byte = 0xa2;
  else if (clt>=95) Byte = 0x9f;
  else if (clt>90) Byte = 0x9d;
  else if (clt>85) Byte = 0x70;
  else if (clt>80) Byte = 0x64;
  else if (clt>70) Byte = 0x5a;
  else if (clt>60) Byte = 0x50;
  
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
  //0x06214000 FUEL
  Byte = 0;
  if((getCurrentMillis()-dimmer_time)<5000) //have  valid sensor frame
  {
      if (F_BCL) Byte+=BCL_EN;
  }
  else
  {
      if (F_SIDELAMP) Byte+=BCL_EN;
  }
  
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

  if (first_run) 
  filtered_fuel_level = data.fuel_level; // diasble Exponential moving average for first reading
  else
  {
    filtered_fuel_level = alfa*fuel + (1-alfa)*filtered_fuel_level;
    data.fuel_level = (uint8_t)filtered_fuel_level;
  } 
  Byte = 0;
  if (filtered_fuel_level <= 10) Byte+=LOW_FUEL_BLINK;// less than 9 liters or 15% - float stops floating at this point
  else if (filtered_fuel_level <= 15) Byte+=LOW_FUEL_IND; // less than 12 liters or 20% 
  if (first_run) Byte = 0;
  Data[4] = Byte;
  
  Data[5] = filtered_fuel_level;

  //Byte = 0;
  //if (F_LOCK) Byte+=LOCK_IND;
  //if (F_LOCK_BLINK) Byte+=LOCK_IND_BLINK;
  Data[6] = 0;
  Data[7] = 0;
  CanSend(0x06214000, Data, 8);

  //0x04394000 Speedometer
  //tripCalculate();
  uint16_t speed_tmp = speed*14.9;
  Data[0] = hi8(speed_tmp);
  Data[1] = lo8(speed_tmp);
  Data[2] = 0;
  Data[3] = odo_cnt;
  CanSend(0x04394000, Data, 4);
  //0x0621401A Belt stuff. Die like a real man
  //Used as shift indicator
      Byte = 0;
      uint8_t F_shift;
      if (rpm>5000 && first_run ==0)
      {
        Byte+=PASS_AIRBAG_BLINK;
      }
      if (rpm>5250 && first_run ==0)
      {
        Byte+=AIRBAG_BLINK;
      }
      
      //if (F_AIRBAG_BLINK) Byte+=AIRBAG_BLINK;
      //if (F_AIRBAG) Byte+=AIRBAG_FAIL;
      //if (F_PASS_AIRBAG_BLINK) Byte+=PASS_AIRBAG_BLINK;
      //if (F_PASS_AIRBAG) Byte+=PASS_AIRBAG;
      Data[0] = Byte;
      Byte = 0;
      if (rpm>5500 && first_run ==0) 
      Byte+= BELT;
      Data[1] = Byte;
      Data[2] = Byte;
      Data[3] = 0;
      Data[4] = 0;
      Data[5] = 0;
      Data[6] = 0;
      Data[7] = 0;
      CanSend(0x0621401A, Data, 8);

  
  
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
  CanSend(0x0E094000,status2,6); //send status request
  unsigned char Data[8];
  unsigned char Byte; 

    
      //0x06314000 Charge
      Byte = 0;
      if (rpm<200) 
      {
        if (voltage>115) Byte+=CHARGE;
        else Byte+= CHARGE_BLINK;
      }
      else
      {
        if (voltage<115) Byte+=CHARGE_BLINK;
        else if (voltage<125) Byte+=CHARGE;
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

    

      //0x063D4000 outside temp (T+40)*2 FROM -39 to 88 
      if (outside_temp>88) outside_temp = 88;
      Byte = (outside_temp+40)*2;
      Data[0] = Byte;
      Data[1] = 0;
      Data[2] = 0;
      Data[3] = 0;
      CanSend(0x063D4000, Data, 4);
    first_run = false;
}

void tripCalculate (uint16_t time)
{
  //static uint32_t time = millis()-time;
  
  float increment   =   (speed / 3600.0)*(time);
  //                    =   m/s       * (millisec)
	odo_tmp += increment;
  deltaTrip = increment;
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
      ValueToText(uint8_t(showErr));
      CanText(msgBuff);      
    }

    if (msgTick>ERROR_TICKS){
      if (pendingMsg>0)
      {
        showErr = !showErr;
      }
      else
      {
        showErr = false;
      }
      msgTick=0;
    }
    msgTick++;
  
}

void ValueToText (uint8_t show_error){
 uint8_t msg;
  if (show_error==0) msg = data.currentMsg;
  else msg = pendingMsg;
  switch(msg){
    //info messages
    case msg_tripA:
      trip_counter = (int)(data.tripMeter/100);
      sprintf(msgBuff,"TRIP%6d.%dKM", trip_counter/10, trip_counter%10); //sprinf with floats uses shit ton of resources
      break;
    case msg_tripAVG:
    //AVG##15.5L/100
      sprintf(msgBuff,"AVG%4d.%dL/100", avgF/10, avgF%10); //sprinf with floats uses shit ton of resources
      break;
      /*case msg_Range:
      {
    //RANGE####100KM
    uint16_t rng = ((0.6*fuel)/(avgF/10))*100;
      if (rng>2000 || avgF<40) rng = 0;  //probably bullshit value
      sprintf(msgBuff,"RANGE%7dKM", rng); // convert fuel in % to litters  / divide by avg consumtion per 100 * multiplied by 100
      break;
      }
      /*case msg_Inst:
      {
    //INST#15.5L/100
    //INST##15.5L/HR
    uint16_t inst = int(instF*10);
    if (speed>0)
      sprintf(msgBuff,"INST%3d.%dL/100", inst/10, inst%10); // convert fuel in % to litters  / divide by avg consumtion per 100 * multiplied by 100
    else
      sprintf(msgBuff,"INST%4d.%dL/HR", inst/10, inst%10);
      break;
      }*/
    case msg_BattV:
      sprintf(msgBuff,"BATT%7d.%dV", voltage/10, voltage%10);
      break;
    /*case msg_CltT:
      //sprintf(msgBuff,"CLT%9doC", clt);
      sprintf(msgBuff,"COOLANT%5doC", clt);
      break;      
    case msg_OilT:
      sprintf(msgBuff,"OIL T%7doC", oilt);
      break;*/
    case msg_TempMsg:
      //sprintf(msgBuff,"CLT%9doC", clt);
      sprintf(msgBuff,"O%3doC  C%3doC", oilt, clt);
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
  CanSend(0x0E094000,wakeup,6);
  delay(100);
  CanSend(0x0E094000,status1,6);
  speed = 0;
  rpm = 0;
  fuel = data.fuel_level;
  ClusterFramesSend();
  delay(50);
  speed = 230;
  rpm = 7500;
  fuel = data.fuel_level;
  ClusterFramesSend();
  delay(sweep_time);
  speed = 0;
    rpm = 0;
    fuel = data.fuel_level;
  ClusterFramesSend();
  delay(50);
  can.attachInterrupt(canISR);
}

