#ifndef STORAGE_H
#define STORAGE_H

#include<arduino.h>
#include "serialcomm.h"

#define MEM_SIZE 32768
#define PAGE_SIZE 64
#define BLOCK_SIZE 64 //in pages
#define PAGE_IN_BLOCK BLOCK_SIZE/PAGE_SIZE
#define BLOCK_NUM 400
#define NON_WL_START 1 + (BLOCK_SIZE *BLOCK_NUM) //25601
#define NON_WL_SIZE MEM_SIZE - NON_WL_START //7167

extern bool eeprom_corrupt;
void initializeStorage (void);
uint8_t getAddr (void);
void writeByte(uint8_t deviceaddress, uint16_t eeaddress, byte data);
void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer);
byte readByte(uint8_t deviceaddress, uint16_t eeaddress);
void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer);
byte checksumCalculate (uint8_t *buffer, uint8_t length);
byte checksumCheck (uint8_t *buffer, uint8_t length);
uint16_t WLfindFreshBlock(void);
void readBlock(void);
void getDataFromBlock (uint8_t *buffer);

#endif