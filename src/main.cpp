
#include <arduino.h>
#include "Can.h"
#include "storage.h"
#define LED PC13
#define TIMEFRAME 200
#define EEPROM_TIME 15000/TIMEFRAME //15s


uint32_t ms;
uint16_t ee_tick;





void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    delay(500);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);   // This one was missing... 
    InitializeCan();
    initializeStorage();
}


void loop()
{   
    
    ms = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    while (1)
    {
        if (millis() - ms >= TIMEFRAME)
        {
          ee_tick++;
          break;
        } 
    }
    if (ee_tick>=EEPROM_TIME)
    {
      ee_tick = 0;
      storeData();
    }
    

}

