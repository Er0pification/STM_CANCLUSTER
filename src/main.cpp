
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
#define TIMEFRAME 50 // should be 200
#define EEPROM_TIME 15000/TIMEFRAME //15s


uint32_t ms;
uint16_t ee_tick;





void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    //SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);   // This one was missing... 
    //enableBackupDomain();
    InitializeCan();
    ClrText();
    initializeStorage();
    initialize_inputs();
    /*setBackupRegister(3,0x1488);
    setBackupRegister(1,0x69);
    setBackupRegister(2,0x420);*/
    
    
  
  SweepIndicators();
}

void loop()
{
  ms = millis();
  if (millis() - ms >= TIMEFRAME)
  {
    ee_tick++;
    // serialGetData();
    ClusterFramesSend();
    UpdateText();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    ms = millis();
  }
  if (ee_tick >= EEPROM_TIME)
  {
    ee_tick = 0;
    storeData();
  }
  read_inputs();
  delay(50);
}
