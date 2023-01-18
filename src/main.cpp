
#include <arduino.h>
//#include <eXoCAN.h>
#include <backup.h>  
//#include "fiat_msg.h"
//#include "flags.h"
//#include "base64.h"
//#include "Screenmsg.h"
#include "Can.h"
#include "serialcomm.h"
#define LED PC13
//(Speed and RPM)
// SPEED CALCULATION FUNCTIONS








void setup()
{

    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);   // This one was missing... 
    enableBackupDomain();
    InitializeCan();
    InitializeSerial();
    /*setBackupRegister(3,0x1488);
    setBackupRegister(1,0x69);
    setBackupRegister(2,0x420);*/
    pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  SweepIndicators();
}


void loop()
{   
    
    
    
    ClusterFramesSend();
    UpdateText();
    delay(50);

}

