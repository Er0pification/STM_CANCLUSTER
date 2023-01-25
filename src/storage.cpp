#include "storage.h"

#include <Wire.h>

uint8_t addr;
bool eeprom_corrupt = false;
uint16_t block_num;
uint8_t block_write_cnt;

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

void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer)
{
Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  for (uint8_t i = 0; i<64; i++){
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

void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer) 
{  
  Wire.beginTransmission(deviceaddress);
  Wire.write(eeaddress >> 8);
  Wire.write(eeaddress & 0xFF);
  Wire.endTransmission(); 

  Wire.requestFrom(deviceaddress,1); 

  //if (Wire.available()) 
  buffer[0] = Wire.read();
  Wire.requestFrom(deviceaddress,1);
  for (uint8_t i = 1; i<64; i++){
        buffer[i] = Wire.read();
    }
}

byte checksumCalculate (uint8_t *buffer, uint8_t length)
{
    uint16_t checksum;
    uint8_t checksum8;
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
    for (uint16_t i = 0; i<=BLOCK_NUM; i++)
    {
        block_write_cnt = readByte(addr,i*BLOCK_SIZE);
        if (block_write_cnt<255)
        {
            block_num = i*BLOCK_SIZE;
            return block_num;
        }
    }
    //if  end of wear-levelled memory area is reached, but no "fresh" block was found -> something is wrong, eeprom may be corrupted;
    eeprom_corrupt = true;
    return 0xffff;
}

void readBlock(void)
{
    uint8_t buffer[BLOCK_SIZE];
    for(uint8_t i = 0; i<PAGE_IN_BLOCK; i++)
    {
        readPage(addr,i*PAGE_SIZE + block_num * BLOCK_SIZE, buffer);
    }
    if (checksumCalculate(buffer,BLOCK_SIZE)) //checksum check
    {
            getDataFromBlock(buffer);
    }
    else  //if checksum error
    {
        eeprom_corrupt= true;
    }
}

void getDataFromBlock (uint8_t *buffer)
{
    block_write_cnt = buffer[0];
    //TODO - DOIT!
}