/*
##############
TRIP##0000.0KM
AVG##15.5L/100
INST#15.5L/100
INST##15.5L/HR
BATT#####14.5V
CLT######100oC
OILT#####100oC
BOOST####68KPA
*/


#ifndef CAN_H
#define CAN_H

#include <eXoCAN.h>
#include "lcd.h"

#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)




void CanISR(void);
void InitializeCan(void);
void CanSend(long, unsigned char *, char );
#endif