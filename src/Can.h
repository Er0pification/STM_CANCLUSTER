/*
##############
TRIP##0000.0KM
AVG##15.5L/100
BATT#####14.5V
CLT######100oC
OILT#####100oC
BOOST####68KPA
*/


#ifndef CAN_H
#define CAN_H

#include <eXoCAN.h>
#include "base64.h"
#include "storage.h"

#define sweep_time 2000
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)

extern uint8_t currentMsg, prevMsg;
extern int clt;
extern int oilt;
extern int avgF;
extern int voltage;
extern uint16_t rpm;
extern uint16_t speed;
extern uint8_t fuel;
extern int trip_counter;
 extern int outside_temp;
enum msgtype {Def, msg_tripA, msg_tripAVG,  msg_BattV, msg_TempMsg, msg_WMI_LOW, msg_WMI_EMPTY, msg_WMI_FAIL, MsgNum}; //msg_Inst, msg_Range,  msg_CltT, msg_OilT,
#define LAST_MSG msg_TempMsg
extern bool cluster_awake;
extern uint8_t odo_cnt;



void CanISR(void);
void InitializeCan(void);
void SweepIndicators (void);
void ClusterFramesSend (void);
void ClusterSlowFramesSend (void);
void ClusterFastFramesSend (void);
void CanSend(long, unsigned char *, char );
void CanText (char *);
void ClrText (void);
void UpdateText (void);
void ValueToText (uint8_t);
void NextMsg (void);
void fuelCalc (void);
void tripCalculate(uint16_t);
void tripReset (void);
void resetFlags (void);
bool NoValidData (void);

#endif