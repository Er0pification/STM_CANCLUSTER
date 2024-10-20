#include "storage.h"

#include <Wire.h>
//#define AT24C16

uint8_t addr;
bool eeprom_corrupt = false;
uint16_t block_num;
uint8_t block_write_cnt;

dataUnion_t data;

void initializeStorage (void)
{
    Wire.setSDA(PB9);
    Wire.setSCL(PB8);
    Wire.begin();
    if (getAddr()) 
    {
        //testStorage();
        //wipeStorage();
        if (WLfindFreshBlock()!=0xffff)
        {
            readBlock();
        }
    }
    else
    {
         #ifdef HASLCD
            lcd_terminal("No EEPROM found!",0);
        #endif
        
        eeprom_corrupt = true;
    }
    
}

uint8_t getAddr (void)
{
        addr = 174;
        while (addr<=255)
        {
            Wire.beginTransmission(addr);
            uint8_t resp = Wire.endTransmission();
            if (resp == 0) {
                return addr;
            }
            
            addr++;
        }
        addr = 0;
        return 0; //nothing found
}

void writeByte(uint8_t deviceaddress, uint16_t eeaddress, byte data) 
{
    #ifdef AT24C16
        Wire.beginTransmission(deviceaddress | ((eeaddress >> 7) & 0xE));
        delayMicroseconds(25);
        Wire.write((uint8_t)(eeaddress & 0xFF));
        delayMicroseconds(25);
        Wire.write(data);
        delayMicroseconds(25);
        Wire.endTransmission();
    #else
        delayMicroseconds(5);
        Wire.beginTransmission(deviceaddress);
        Wire.write((uint8_t)(eeaddress >> 8));
        Wire.write((uint8_t)(eeaddress & 0xFF));
        Wire.write(data);
        Wire.endTransmission();
        delayMicroseconds(5);
    #endif
    
}

void writePage (uint8_t deviceaddress, uint16_t eeaddress, uint8_t *buffer, uint8_t length)
{
   #ifdef AT24C16
        Wire.beginTransmission(deviceaddress | ((eeaddress >> 7) & 0x0E));
        delayMicroseconds(25);
        Wire.write((uint8_t)(eeaddress & 0xFF));
        delayMicroseconds(25);
    #else
        delayMicroseconds(5);
        Wire.beginTransmission(deviceaddress);
        Wire.write((uint8_t)(eeaddress >> 8));
        Wire.write((uint8_t)(eeaddress & 0xFF));
    #endif
    for (uint8_t i = 0; i<length; i++){
            Wire.write(buffer[i]);
            delayMicroseconds(10);
    }
    
    Wire.endTransmission();
    delayMicroseconds(25);
}

void wipePage (uint8_t deviceaddress, uint16_t eeaddress,  uint8_t length)
{
    #ifdef AT24C16
        Wire.beginTransmission(deviceaddress | ((eeaddress >> 7) & 0x0E));
        delayMicroseconds(25);
        Wire.write((uint8_t)(eeaddress & 0xFF));
        delayMicroseconds(25);
    #else
        delayMicroseconds(5);
        Wire.beginTransmission(deviceaddress);
        Wire.write((uint8_t)(eeaddress >> 8));
        Wire.write((uint8_t)(eeaddress & 0xFF));
    #endif
    for (uint8_t i = 0; i<length; i++){
            Wire.write(0);
            delayMicroseconds(10);
    }
    
    Wire.endTransmission();
    delayMicroseconds(25);
}

 

byte readByte(uint8_t deviceaddress, uint16_t eeaddress) 
{

  byte rdata = 0xFF; 
  #ifdef AT24C16
        Wire.beginTransmission(deviceaddress | ((eeaddress >> 7) & 0x0E)  | 0x01);
        delayMicroseconds(25);
        Wire.write((uint8_t)(eeaddress & 0xFF));
        delayMicroseconds(25);
        Wire.endTransmission();
    #else
        delayMicroseconds(5);
        Wire.beginTransmission(deviceaddress);
        Wire.write((uint8_t)(eeaddress >> 8));
        Wire.write((uint8_t)(eeaddress & 0xFF));
        Wire.endTransmission();
        delayMicroseconds(5);
    #endif

  Wire.requestFrom(deviceaddress,1); 

  if (Wire.available()) rdata = Wire.read();

 

  return rdata;

}

void readPage(uint8_t deviceaddress, uint16_t eeaddress, uint8_t * buffer, uint8_t length) 
{  
   #ifdef AT24C16
        Wire.beginTransmission(deviceaddress | ((eeaddress >> 7) & 0x0E)  | 0x01);
        delayMicroseconds(25);
        Wire.write((uint8_t)(eeaddress & 0xFF));
        delayMicroseconds(25);
        Wire.endTransmission();
    #else
        delayMicroseconds(5);
        Wire.beginTransmission(deviceaddress);
        Wire.write((uint8_t)(eeaddress >> 8));
        Wire.write((uint8_t)(eeaddress & 0xFF));
        Wire.endTransmission();
        delayMicroseconds(5);
    #endif

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
    /*char buf[23];
        sprintf(buf, "CRC =  0x%02x",checksum8);
        lcd_terminal(buf,0);
        sprintf(buf, "CRC stored =  0x%02x",buffer[length-1]);
        lcd_terminal(buf,0);*/
    uint8_t result = (checksum8- buffer[length-1]); //error check
    //buffer[length-1] = ~checksum8; //store 1’s complement of checksum to array
    buffer[length-1] = checksum8;
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
     #ifdef HASLCD
        lcd_terminal("No fresh block found...",0);
        lcd_terminal("EEPROM corrupted!",0);
        lcd_terminal("Wiping EEPROM",0);
    #endif
    
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
         #ifdef HASLCD
            lcd_terminal("EEPROM READ OK!",0);
        #endif
        
    }
    else  //if checksum error
    {

         #ifdef HASLCD
            lcd_terminal("EEPROM CRC FAIL!",0);
        #endif
       
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
     #ifdef HASLCD
        lcd_terminal("EEPROM wiped!",0);
    #endif
    
}

byte testStorage (void)
{
    writeByte(addr, 2, 0x42);
    delay(10);
    byte result = readByte(addr,2);
     #ifdef HASLCD
        char buf[23];
        sprintf(buf, "EEPROM test=0x%02x",result);
        lcd_terminal(buf,0);
    #endif
    
}

