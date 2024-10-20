
#include <arduino.h>
#include "Can.h"
#include "storage.h"
#include "hardware.h"
 #ifdef HASLCD
    #include "lcd.h"
  #endif

#define LED PC13
#define TIMEFRAME 1000
#define EEPROM_TIME 15000/TIMEFRAME //15s


uint32_t ms;
uint16_t ee_tick;





void setup()
{

    pinMode(LED, OUTPUT);
    //digitalWrite(LED, HIGH);
    #ifdef HASLCD
      lcd_setup();
      lcd_terminal("Initializing...",0);
    #endif
    
    InitializeCan();
    initializeStorage();
    initialize_hardware();
    
}


void loop()
{   
    ms = millis();
    
    while (1)
    {
      uint32_t mill = HAL_GetTick() ;
        if (mill - ms >= TIMEFRAME)
        {
          ee_tick++;
          set_outputs();
          //read_inputs();
          digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
          break;
          
        } 
    }
   
    byte buf[8];
    buf[0] = ms/10;
    CanSend(0x420, buf, 8);
    if (ee_tick>=EEPROM_TIME)
    {
      ee_tick = 0;
      data.dataVersion = 60;
      storeData();
    }
    

}

