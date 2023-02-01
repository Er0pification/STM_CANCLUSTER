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
#include "serialcomm.h"
#include "base64.h"

#define sweep_time 1000
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)

extern uint8_t currentMsg, prevMsg;
extern int clt;
extern int oilt;
extern int avgF;
extern int instF;
extern int voltage;
extern uint16_t rpm;
extern uint8_t speed;
extern uint8_t fuel;
extern uint8_t fuel_ltr;
extern uint8_t fuel_capacity;
extern uint8_t fuel_low_level;
extern int trip_counter;
 extern int outside_temp;
enum msgtype {Def, msg_tripA, msg_tripL, msg_AvgCons, msg_InstCons, msg_BattV, msg_CltT, msg_OilT, msg_Timeout, msg_Corrupt, msg_EECorrupt, MsgNum};
#define LAST_MSG msg_OilT
extern bool cluster_awake;
extern uint8_t odo_cnt;



void CanISR(void);
void InitializeCan(void);
void SweepIndicators (void);
void ClusterFramesSend (void);
void CanSend(long, unsigned char *, char );
void CanText (char *);
void ClrText (void);
void UpdateText (void);
void ValueToText (void);
void NextMsg (void);

#endif