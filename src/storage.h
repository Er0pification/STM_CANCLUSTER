#ifndef STORAGE_H
#define STORAGE_H

#include<arduino.h>
#include "lcd.h"

#define MEM_SIZE 262144
#define PAGE_SIZE 64
#define BLOCK_SIZE 64 
#define PAGE_IN_BLOCK 1
#define BLOCK_NUM 400
#define NON_WL_START 1 + (BLOCK_SIZE *BLOCK_NUM) //12801
#define NON_WL_SIZE MEM_SIZE - NON_WL_START //3584

typedef union{
    uint8_t bytearray[BLOCK_SIZE];
    struct{
        uint8_t WL_cnt; //1
        uint8_t dataVersion; //2
        uint8_t unused0; //3
        uint8_t unused1; //4
        uint8_t unused2; //5
        uint8_t unused3; //6
        uint8_t unused4; //7
        uint8_t unused5; //8
        uint8_t unused6; //9
        uint8_t unused7; //10
        uint8_t unused8; //11
        uint8_t unused9; //12
        uint8_t unused10; //13
        uint8_t unused11; //14
        uint8_t unused12; //15
        uint8_t unused13; //16
        uint8_t unused14; //17
        uint8_t unused15; //18
        uint8_t unused16; //19
        uint8_t unused17; //20
        uint8_t unused18; //21
        uint8_t unused19; //22
        uint8_t unused20; //23
        uint8_t unused21; //24
        uint8_t unused22; //25
        uint8_t unused23; //26
        uint8_t unused24; //27
        uint8_t unused25; //28
        uint8_t unused26; //29
        uint8_t unused27; //30
        uint8_t unused28; //31
        uint8_t unused29; //32
        uint8_t unused30; //33
        uint8_t unused31; //34
        uint8_t unused32; //35
        uint8_t unused33; //36
        uint8_t unused34; //37
        uint8_t unused35; //38
        uint8_t unused36; //39
        uint8_t unused37; //40
        uint8_t unused38; //41
        uint8_t unused39; //42
        uint8_t unused40; //43
        uint8_t unused41; //44
        uint8_t unused42; //45
        uint8_t unused43; //46
        uint8_t unused44; //47
        uint8_t unused45; //48
        uint8_t unused46; //49
        uint8_t unused47; //50
        uint8_t unused48; //51
        uint8_t unused49; //52
        uint8_t unused50; //53
        uint8_t unused51; //54
        uint8_t unused52; //55
        uint8_t unused53; //56
        uint8_t unused54; //57
        uint8_t unused55; //58
        uint8_t unused56; //59
        uint8_t unused57; //60
        uint8_t unused58; //61
        uint8_t unused59; //62
        uint8_t unused60; //63
        uint8_t checksum; //64
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
byte testStorage (void);
#endif