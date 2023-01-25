
#include <arduino.h>
//#include <eXoCAN.h>
#include <backup.h>  
//#include "fiat_msg.h"
//#include "flags.h"
//#include "base64.h"
//#include "Screenmsg.h"
#include "Can.h"
#include "serialcomm.h"
#include "storage.h"
#define LED PC13
//(Speed and RPM)
// SPEED CALCULATION FUNCTIONS








void setup()
{
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    delay(500);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);   // This one was missing... 
    enableBackupDomain();
    InitializeCan();
    ClrText();
    InitializeSerial();
    initializeStorage();
    /*setBackupRegister(3,0x1488);
    setBackupRegister(1,0x69);
    setBackupRegister(2,0x420);*/
    trip_counter = 1488;
    
  
  SweepIndicators();
}


void loop()
{   
    
    
    serialGetData();
    ClusterFramesSend();
    UpdateText();
    trip_counter +=1;
    delay(100);
    if (no_resp) DbgSerial.println("ECU Response timeout!");
    else DbgSerial.println("Serial OK");
    

}

