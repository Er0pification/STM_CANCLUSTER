#ifndef STORAGE_H
#define STORAGE_H

#include<arduino.h>

#define MEM_SIZE 32768
#define PAGE_SIZE 64
#define BLOCK_SIZE 32 
#define PAGE_IN_BLOCK 1
#define BLOCK_NUM 400
#define NON_WL_START 1 + (BLOCK_SIZE *BLOCK_NUM) //25601
#define NON_WL_SIZE MEM_SIZE - NON_WL_START //7167

typedef union{
    uint8_t bytearray[BLOCK_SIZE];
    struct{
        uint8_t WL_cnt; //1
        uint8_t dataVersion; //2
        uint8_t currentMsg; //3
        uint8_t odo_cnt; //4
        float tripMeter; //8
        double CumulativeFuel; //16,  in cc
        uint8_t pendingMSG; //17
        uint8_t unused2; //18
        uint8_t unused3; //19
        uint8_t unused4; //20
        uint8_t unused5; //21
        uint8_t unused6; //22
        uint8_t unused7; //23
        uint8_t unused8; //24
        uint8_t unused9; //25
        uint8_t unused10; //26
        uint8_t unused11; //27
        uint8_t unused12; //28
        uint8_t unused13; //29
        uint8_t unused14; //30
        uint8_t unused15; //31
        uint8_t checksum; //32
    };
} dataUnion_t;

extern  dataUnion_t data;

extern bool eeprom_corrupt;
void initializeStorage (void);
uint8_t getAddr (void);
void writeByte(uint8_t deviceaddress, uint16_t eeaddress, byte data);
void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer, uint8_t length);
void wipePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t length);
byte readByte(uint8_t deviceaddress, uint16_t eeaddress);
byte readByte(uint8_t deviceaddress, uint16_t eeaddress);
void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer, uint8_t length);
byte checksumCalculate (uint8_t *buffer);
uint16_t WLfindFreshBlock(void);
void readBlock(void);
void storeData(void);
void wipeStorage(void);

#endif