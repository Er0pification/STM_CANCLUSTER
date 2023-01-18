#ifndef SERIALCOMM_H
#define SERIALCOMM_H
#include <arduino.h>
#include "Can.h"

#define SERIAL_TIMEOUT 250
#define COMM_GET_LENGTH 'l'
#define COMM_REQUEST_PACKET 'n'
#define MAX_PACKET_LENGTH 30

#define CALIBRATION_TEMPERATURE_OFFSET 40

#define INDEX_CLT 4+2
#define INDEX_OILT 16+2
#define INDEX_VOLTAGE 5+2
#define INDEX_RPM 7+2
#define INDEX_VSS 13+2
#define INDEX_FUEL 17+2
#define INDEX_AMBIENT 15+2
#define INDEX_SPARK 11+2
#define LIMITER_MASK 0b11111000
#define SYNC_MASK 0b00000001


extern bool data_valid;

extern uint8_t status_engine;
extern uint16_t MAP;
extern int8_t IAT;
extern int8_t CLT;
extern uint8_t battCorrection;
extern uint8_t battery10;
extern uint8_t O2;
extern uint16_t RPM;
extern uint16_t PW;
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

void InitializeSerial(void);
char sendCommand (char);
char serialWait (void);
char serialRequestPacket(void);
char serialGetPacket (void);
void setFlags (void);

#endif