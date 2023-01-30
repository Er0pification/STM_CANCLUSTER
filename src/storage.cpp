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
    DbgSerial.println("Initializing EEPROM");
    if (getAddr()) 
    {
        DbgSerial.print("Device EEPROM at ");
        DbgSerial.printf("0x%2X", addr);
        DbgSerial.println(" ");
        if (WLfindFreshBlock()!=0xffff)
        {
            readBlock();
        }
    }
    else
    {
        DbgSerial.print("No EEPROM found!");
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
    DbgSerial.println("No Device found!");
    return 0; //nothing found
    
}

void writeByte(uint8_t deviceaddress, uint16_t eeaddress, byte data) 
{

  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  Wire.write(data);
  Wire.endTransmission();
}

void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer, uint8_t length)
{
Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  for (uint8_t i = 0; i<length; i++){
        Wire.write(buffer[i]);
  }
  
  Wire.endTransmission();
}

 

byte readByte(uint8_t deviceaddress, uint16_t eeaddress) 
{

  byte rdata = 0xFF; 
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  Wire.endTransmission(); 

  Wire.requestFrom(deviceaddress,1); 

  if (Wire.available()) rdata = Wire.read();

 

  return rdata;

}

void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer, uint8_t length) 
{  
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  Wire.endTransmission(); 

  Wire.requestFrom(deviceaddress,length); 

  //if (Wire.available()) 
  //buffer[0] = Wire.read();
  //Wire.requestFrom(deviceaddress,1);
  for (uint8_t i = 0; i<length; i++){
        if (Wire.available()) buffer[i] = Wire.read();
       
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
    checksum8 = ~checksum8; //1’s complement
    buffer[length-1] = checksum8; //store checksum to array
    if (result == 0) return 1; //if error check result is NULL -> no error occured
    else return 0;
}

uint16_t WLfindFreshBlock(void)
{
    for (uint16_t i = 0; i<BLOCK_NUM; i++)
    {
        block_write_cnt = readByte(addr,i*BLOCK_SIZE);
        if (block_write_cnt<255)
        {
            block_num = i*BLOCK_SIZE;
            DbgSerial.printf("Wear levelling - reading block no. %d\n", block_num);
            return block_num;
        }
    }
    //if  end of wear-levelled memory area is reached, but no "fresh" block was found -> something is wrong, eeprom may be corrupted;
    DbgSerial.println("Wear levelling ERROR! No fresh blocks found!");
    DbgSerial.println("Wiping EEPROM...");
    wipeStorage();
    eeprom_corrupt = true;
    return 0xffff;
}

void readBlock(void)
{
    for(uint8_t i = 0; i<PAGE_IN_BLOCK; i++)
    {
        readPage(addr,i*PAGE_SIZE + block_num * BLOCK_SIZE, data.bytearray, BLOCK_SIZE);
    }
    if (checksumCalculate(data.bytearray)) //checksum check
    {   
        DbgSerial.println("Checksum validation passed.");
        data.WL_cnt++;
        DbgSerial.printf("Wear levelling - block writes count = %d\n", data.WL_cnt);
        if(data.WL_cnt==255)//if block is toasted, prepare to switch for next
        {
            writeByte(addr,block_num*BLOCK_SIZE,255);
            if(block_num<BLOCK_NUM-1) //check if next block is available
            {
                block_num++;
            }
            else //starting from beginning
            {
                block_num = 0;
            }
            DbgSerial.printf("Wear levelling - block retiring, switching to block no. %d\n", block_num);
        }
    }
    else  //if checksum error
    {
        DbgSerial.println("Checksum validation FAILED!");
        eeprom_corrupt= true;
    }
}

void storeData(void)
{
    checksumCalculate(data.bytearray);
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
    for(uint8_t i = 0; i<PAGE_IN_BLOCK; i++)
    {
        writePage(addr,i*PAGE_SIZE + block_num * BLOCK_SIZE, data.bytearray, BLOCK_SIZE);
    }
    DbgSerial.printf("Wear levelling - block %d writes count = %d\n", block_num, data.WL_cnt);
    
}

void wipeStorage (void)
{
    byte nullBuffer[PAGE_SIZE];
    for (uint16_t i = 0; i< BLOCK_NUM*PAGE_IN_BLOCK; i++)
    {
        for (uint8_t j = 0; j< PAGE_IN_BLOCK; j++)
    {
        writePage(addr,i*j*PAGE_SIZE,nullBuffer,PAGE_SIZE);
    }
    DbgSerial.printf("EEPROM block %d wiped\n",i);
    }
}

