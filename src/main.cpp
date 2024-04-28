
#include <arduino.h>
//#include <eXoCAN.h>
#include <backup.h>  
//#include "fiat_msg.h"
//#include "flags.h"
//#include "base64.h"
//#include "Screenmsg.h"
#include "Can.h"
//#include "serialcomm.h"
#include "storage.h"
#include "hardware.h"
#define LED PC13
#define TIMEFRAME 200
#define EEPROM_TIME 15000/TIMEFRAME //15s
#define MSG_TIMEFRAME 500
#define INP_TIMEFRAME 20

uint32_t ms, ms_msg, loop_time;
uint16_t ee_tick;
uint8_t frametick;





void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    InitializeCan();
    digitalWrite(LED, HIGH);
    ClrText();
    initializeStorage();
    initialize_inputs();  
  SweepIndicators();
  ms = millis();
  ms_msg = ms;
}

void loop()
{ 
  ClusterFastFramesSend();
  loop_time = millis() - ms;
  if (loop_time >= TIMEFRAME)
  {
    ee_tick++;
    frametick++;    
    tripCalculate(loop_time);
      fuelCalc();
      ClusterFramesSend();
    if (frametick == 2)
    {
        ClusterSlowFramesSend();
        frametick = 0;
      }
    
    //digitalWrite(LED, !digitalRead(LED));
    ms = millis();
    
  }
  if (millis()-ms_msg >= MSG_TIMEFRAME)
  {
    if(btn_press)
    {
      NextMsg();
      btn_press--;
    }
    else if (btn_longpress)
    {
      tripReset();
      data.currentMsg = msg_tripA;
      btn_longpress = false;
    }
    UpdateText();
    ms_msg = millis();
  }
  if (ee_tick >= EEPROM_TIME)
  {
    ee_tick = 0;
    storeData();
  }
  read_inputs();
  delay(50);
}