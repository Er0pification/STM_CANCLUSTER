#include "storage.h"

#include <Wire.h>

uint8_t addr;
bool eeprom_corrupt = false;
uint16_t block_num;
uint8_t block_write_cnt;

dataUnion_t data;

void initializeStorage (void)
{
    Wire.setSDA(PB11);
    Wire.setSCL(PB10);
    Wire.begin();
    if (getAddr()) 
    {
        
        //wipeStorage();
        if (WLfindFreshBlock()!=0xffff)
        {
            readBlock();
        }
    }
    else
    {
        
        eeprom_corrupt = true;
    }
    
}

uint8_t getAddr (void)
{
    addr = 0;
    while (addr<=127)
    {
        Wire.beginTransmission(addr);
        uint8_t resp = Wire.endTransmission();
        if (resp == 0) return addr;
        addr++;
    }
    addr = 0;
    return 0; //nothing found
    
}

void writeByte(uint8_t deviceaddress, uint16_t eeaddress, byte data) 
{

    Wire.beginTransmission(deviceaddress);
    Wire.write((uint8_t)(eeaddress >> 8));
    Wire.write((uint8_t)(eeaddress & 0xFF));
    Wire.write(data);
    Wire.endTransmission();
}

void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer, uint8_t length)
{
    Wire.beginTransmission(deviceaddress);
    Wire.write((uint8_t)(eeaddress >> 8));
    Wire.write((uint8_t)(eeaddress & 0xFF));
    for (uint8_t i = 0; i<length; i++){
            Wire.write(buffer[i]);
    }
    Wire.endTransmission();
}

void wipePage (uint8_t deviceaddress, uint16_t eeaddress,  uint8_t length)
{
    Wire.beginTransmission(deviceaddress);
    Wire.write((uint8_t)(eeaddress >> 8));
    Wire.write((uint8_t)(eeaddress & 0xFF));
  for (uint8_t i = 0; i<length; i++){
        Wire.write(0);
        delayMicroseconds(10);
  }
  
  Wire.endTransmission();
}

 

byte readByte(uint8_t deviceaddress, uint16_t eeaddress) 
{

  byte rdata = 0xFF; 
  Wire.beginTransmission(deviceaddress);
  Wire.write((uint8_t)(eeaddress >> 8));
    Wire.write((uint8_t)(eeaddress & 0xFF));
  Wire.endTransmission(); 

  Wire.requestFrom(deviceaddress,1); 

  if (Wire.available()) rdata = Wire.read();

 

  return rdata;

}

void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer, uint8_t length) 
{  
  Wire.beginTransmission(deviceaddress);
  Wire.write((uint8_t)(eeaddress >> 8));
    Wire.write((uint8_t)(eeaddress & 0xFF));
  Wire.endTransmission(); 

  Wire.requestFrom(deviceaddress,length); 

  //if (Wire.available()) 
  //buffer[0] = Wire.read();
  //Wire.requestFrom(deviceaddress,1);
  for (uint8_t i = 0; i<length; i++){
        while (!Wire.available()); buffer[i] = Wire.read();
    }
}


byte checksumCalculate (uint8_t *buffer)
{
    uint16_t checksum;
    uint8_t checksum8;
    uint8_t length = sizeof buffer;
    for(uint8_t i = 0; i< length - 1; i++)
    {
        checksum+=buffer[i];
    }
    checksum8 = (checksum&0xff) + (checksum>>8); //wrapping carry over around
    uint8_t result = ~(checksum8+buffer[length-1]); //error check
    buffer[length-1] = ~checksum8; //store 1’s complement of checksum to array
    //data.checksum = ~checksum8;
    if (result == 0) return 1; //if error check result is NULL -> no error occured
    else    return 0;
}

uint16_t WLfindFreshBlock(void)
{
    for (uint16_t i = 0; i<BLOCK_NUM; i++)
    {
        block_write_cnt = readByte(addr,i*BLOCK_SIZE);
        if (block_write_cnt<255)
        {
            block_num = i;
            return block_num;
        }
    }
    //if  end of wear-levelled memory area is reached, but no "fresh" block was found -> something is wrong, eeprom may be corrupted;
    wipeStorage();
    eeprom_corrupt = true;
    return 0xffff;
}

void readBlock(void)
{

    readPage(addr,block_num * BLOCK_SIZE, data.bytearray, BLOCK_SIZE);

    /*DbgSerial.printf("EEPROM read content:\n");
    for(uint8_t i = 0; i<BLOCK_SIZE; i++)
    {
        DbgSerial.printf("%d ", data.bytearray[i]);

    }
    DbgSerial.println("\n");*/
    if (checksumCalculate(data.bytearray)) //checksum check
    {   
        
    }
    else  //if checksum error
    {
        eeprom_corrupt= true;
        //nullify all data except wl counter, sunce it is written first it has most chances to  still be valid
    }
}

void storeData(void)
{
    
    if (data.WL_cnt==255)
    {
        data.WL_cnt = 0;
        if (block_num == BLOCK_NUM - 1) //starting over
        {
            block_num = 0;            
        }
        else
        {
            block_num++;
        }
    }
    else
    {
        data.WL_cnt ++;
    }
        checksumCalculate(data.bytearray);
        writePage(addr,block_num*BLOCK_SIZE, data.bytearray, BLOCK_SIZE);
    
    
}

void wipeStorage (void)
{
    for (uint16_t i = 0; i< BLOCK_NUM; i++)
    {
        wipePage(addr,i*BLOCK_SIZE,BLOCK_SIZE);
    }
    
}

