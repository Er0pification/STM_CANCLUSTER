#ifndef SERIALCOMM_H
#define SERIALCOMM_H
#include <arduino.h>
#include "Can.h"
#include "storage.h"
#include "fiat_msg.h"

#define SERIAL_TIMEOUT 50
#define COMM_GET_LENGTH 'l'
#define COMM_REQUEST_PACKET 'E'
#define COMM_REQUEST_DATA 'r' //New format for the optimised OutputChannels over CAN
//#define COMM_REQUEST_DATA 114 //New format for the optimised OutputChannels over CAN
#define MAX_PACKET_LENGTH 30

#define CALIBRATION_TEMPERATURE_OFFSET 40

#define PREFIX 2  

#define OFF1 4
#define DLC1 18
#define INDEX_MAP 4+ PREFIX -OFF1
#define INDEX_CLT 7+ PREFIX -OFF1
#define INDEX_VOLTAGE 9+ PREFIX -OFF1
#define INDEX_RPM 14+PREFIX-OFF1
#define INDEX_PW 20+PREFIX-OFF1

#define OFF2 41
#define DLC2 9
#define INDEX_AMBIENT 41+PREFIX-OFF2
#define INDEX_OILT 49+PREFIX-OFF2

#define OFF3 100
#define DLC3 2
#define INDEX_VSS 100+PREFIX-OFF3

#define OFF4 122
#define DLC4 3
#define INDEX_FUEL 122+PREFIX-OFF4

#define OFF5 31 //currentStatus.spark;
#define DLC5 1
#define INDEX_SPARK PREFIX
#define LIMITER_MASK 0x0F
#define SYNC_MASK 0x80

#ifndef KM_STEPS
#define ODO_TICK 15.15 //1000m / 0x42
#else
#define ODO_TICK 1000/KM_STEPS //1000m / 0x42
#endif

#define tsCanId 0x00


extern bool data_valid;
extern bool no_resp;

extern uint8_t status_engine;
extern uint16_t MAP;
extern int8_t IAT;
extern int8_t CLT;
extern uint8_t battCorrection;
extern uint8_t battery10;
extern uint8_t O2;
extern uint16_t RPM;
extern float PW;
extern uint8_t status_spark;
extern uint8_t status_protection;
extern uint16_t VSS;
extern uint8_t status_spark;
extern int8_t AMBT;
extern int8_t PreIC_IAT;
extern int8_t IC_cold;
extern int8_t IC_hot;
extern int8_t OILT;
extern uint8_t fuel_percent;
extern uint8_t fuel_capacity;
extern uint8_t fuel_warning;

extern HardwareSerial DbgSerial;
void InitializeSerial(void);
char sendCommand (char);
char serialWait (void);
void serialRequestData(uint16_t, uint16_t);
void serialGetData (void);
void setFlags (void);
void tripCalc (void);
void timeCalc (void);
void fuelCalc (void);
void tripReset (void);
void ping (void);
#endif