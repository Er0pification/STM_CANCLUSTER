

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