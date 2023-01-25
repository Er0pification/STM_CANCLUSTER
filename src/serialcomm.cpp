#include "serialcomm.h"
#include "flags.h"
HardwareSerial Serial2(USART2); 
HardwareSerial DbgSerial(USART1);
uint8_t packet_length = 0;

bool request_success;
bool rev_limiter;
bool sync_loss;
bool no_resp;
bool data_valid;
uint16_t MAP;
uint16_t PW;
uint32_t time;
float trip;
float odo_tmp;
#define ODO_TICK 15.15 //1000m / 0x42

void InitializeSerial(void)
{
    Serial2.begin(115200);
    DbgSerial.begin(115200);
    DbgSerial.println("Serial OK");
    packet_length = sendCommand(COMM_GET_LENGTH);
    if (!packet_length)
    {
        
    }
    else 
    {
        packet_length+=2; // packet starts with 'n' '2'
    }
}

char sendCommand(char command)
{
    Serial2.write(command);
    char response = serialWait();
    if (response == 0)
    {
        DbgSerial.println("Serial to ECU TIMEOUT!");
        no_resp = true;
        return 0;
    }
    return 1;
}

char serialWait(void)
{
    uint8_t cnt;
    while (Serial2.available() == 0)
    {
        delay(5);
        cnt++;
        if (cnt>=SERIAL_TIMEOUT) {
            return 0;
            no_resp = true;
        }
    }
    no_resp= false;
    return (Serial2.available());
}



void serialRequestData (uint16_t offset, uint16_t length)
{
    
    if(sendCommand(COMM_REQUEST_DATA)) request_success = true;
    else {
        request_success = false;
        return;
    }
    sendCommand(tsCanId);
    sendCommand(0x30);
    sendCommand(highByte(offset));
    sendCommand(lowByte(offset));
    sendCommand(highByte(length));
    sendCommand(lowByte(length));
    length+=2;
    char packet[length]; //response should start with 'r' 0x30
    uint8_t index = 0;
    while (index<length)
    {
        //Read the next available byte in the serial receive buffer
        if (!serialWait()) return;
        packet[index++] = Serial.read();
    }
    //sanity check - packet should begin with 'r' 0x30 and be the correct length
    if (packet[0]!='r' || packet[1]!=0x30 || Serial2.available()) 
    {
        data_valid = false;
        return; 
    }
    else  //if everything seems fine - parse the packet
    {   
        data_valid = true;
        switch (offset)
        {
        case OFF1:{ 
            MAP = packet[INDEX_MAP] & (packet[INDEX_MAP+1]<<8);
            clt = packet[INDEX_CLT] - CALIBRATION_TEMPERATURE_OFFSET;
            voltage = packet[INDEX_VOLTAGE];
            rpm = packet[INDEX_RPM] & (packet[INDEX_RPM+1]<<8);
            PW = packet[INDEX_PW] & (packet[INDEX_PW+1]<<8);}
            break;
        case OFF2:{
            outside_temp = packet[INDEX_AMBIENT] - CALIBRATION_TEMPERATURE_OFFSET;
            oilt = packet[INDEX_OILT] - CALIBRATION_TEMPERATURE_OFFSET;}
            break;
        case OFF3:{
            int temp_speed =  packet[INDEX_VSS] & (packet[INDEX_VSS+1]<<8); //speeds beyond 255 kmh are unlikely
            if (temp_speed>255) speed = 255;
            else speed = temp_speed;
            tripCalc();
            }
            break;
        case OFF4:{
            fuel = packet[INDEX_FUEL];
            fuel_capacity = packet[INDEX_FUEL + 1];
            fuel_low_level = packet[INDEX_FUEL + 2];
            fuel_ltr  = fuel_capacity * (fuel / 100.0);}
            break;
        case OFF5:{
            uint8_t status = packet[INDEX_SPARK];
            if(status & LIMITER_MASK)   rev_limiter = true;//this flag is set if ANY rev limiter or spark cut is active
            if(status & SYNC_MASK)      sync_loss = true; }
            break;
        }        
        setFlags();
    }
}

void serialGetData (void)
{
   serialRequestData(OFF1, DLC1);
   serialRequestData(OFF2, DLC2);
   serialRequestData(OFF3, DLC3);
   serialRequestData(OFF4, DLC4);
   serialRequestData(OFF5, DLC5);
   setFlags();
}

void setFlags(void)
{
    if(rpm>500) //consider engine running
    {
        if(voltage<120){F_LOWBATT = true; F_CHARGE = false;}//battery terribly low while running! - blink red indicator
        else if(voltage<129) {F_LOWBATT = false; F_CHARGE = true;} // voltage below 12.9 volts - NO CHARGE
        else {F_LOWBATT = false; F_CHARGE = false;}// charging is fine

        if (rev_limiter){
            F_AIRBAG_BLINK = true;
            F_PASS_AIRBAG_BLINK = true;
            F_GLOW = true;
            F_GENERAL_WARNING = true;
            F_LOCK_BLINK = true;
            }
        else{
            F_AIRBAG_BLINK = false;
            F_PASS_AIRBAG_BLINK = false;
            F_GLOW = false;
            F_GENERAL_WARNING = false;
            F_LOCK_BLINK = false;
            }
        if (sync_loss){
            F_CHECK_ENGINE_BLINK = true;
            F_GENERAL_WARNING = true;
        }
        else{
            F_CHECK_ENGINE_BLINK = false;
            F_GENERAL_WARNING = false;
        }
    }
    else //consider engine not running
    {
        if(voltage<117){F_LOWBATT = true; F_CHARGE = false;}//battery terribly low while not running! - blink red indicator
        else {F_LOWBATT = false; F_CHARGE = true;} // NO CHARGE while not running
        F_OIL_BLINK = true; //won't have any oil pressure for sure
    }

    //not dependent on engine status
    if(fuel_ltr<=fuel_low_level/2) {F_FUEL_VERYLOW = true; F_FUEL_LOW = false;} //if fuel level is very low - flash indicator
    else if(fuel_ltr<=fuel_low_level) {F_FUEL_VERYLOW = false; F_FUEL_LOW = true;} //if fuel level is low - show indicator
    else  {F_FUEL_VERYLOW = false; F_FUEL_LOW = false;}
}

void tripCalc (void)
{
    time = millis() - time;
    float inc = (speed/3.6)*(time/1000); //speed in m/s * time in s
    trip+=inc;
    odo_tmp += inc;
    if (odo_tmp>= ODO_TICK) 
    {
        odo_cnt++;
        odo_tmp-= ODO_TICK;
    }
    time = millis();

}