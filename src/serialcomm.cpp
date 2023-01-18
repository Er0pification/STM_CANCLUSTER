#include "serialcomm.h"
#include "flags.h"
HardwareSerial Serial2(USART2); 
uint8_t packet_length = 0;

bool request_success;
bool rev_limiter;
bool sync_loss;

void InitializeSerial(void)
{
    Serial2.begin(115200);
    packet_length = sendCommand(COMM_GET_LENGTH);
    if (!packet_length)
    {
        //TODO - error handling
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
        CanText("SERIAL TIMEOUT");
        return 0;
    }
    return 1;
}

char serialWait(void)
{
    uint8_t cnt;
    while (Serial2.available() == 0)
    {
        delay(1);
        cnt++;
        if (cnt>=SERIAL_TIMEOUT) return 0;
    }
    return (Serial2.available());
}

char serialRequestPacket (void)
{
    if(sendCommand(COMM_REQUEST_PACKET)) request_success = true;
    else request_success = false;
}

char serialGetPacket (void)
{
    char packet[MAX_PACKET_LENGTH];
    uint8_t index = 0;
    //Check to see if anything is available in the serial receive buffer
    while (Serial.available() > 0)
    {
        //Read the next available byte in the serial receive buffer
        char inByte = Serial.read();
        
        if (index < packet_length )
        {
            //Add the incoming byte to our message
            packet[index] = inByte;
            index++;
        }
    }
    //parse message
    //sanity check - packet should begin with 'n' 0x32 and be the correct length
    if (packet[0]!='n' || packet[1]!=0x32 || index!=packet_length) 
    {
        data_valid = false;
        //return 0; 
    }
    else  //if everything seems fine - parse the packet
    {   
        rev_limiter = false;
        sync_loss = false;
        clt = packet[INDEX_CLT] - CALIBRATION_TEMPERATURE_OFFSET;
        oilt = packet[INDEX_OILT] - CALIBRATION_TEMPERATURE_OFFSET;
        voltage = packet[INDEX_VOLTAGE];
        rpm = packet[INDEX_RPM] & (packet[INDEX_RPM+1]<<8);
        int temp_speed =  packet[INDEX_VSS] & (packet[INDEX_VSS+1]<<8); //speeds beyond 255 kmh are unlikely
        if (temp_speed>255) speed = 255;
        else speed = temp_speed;
        fuel = packet[INDEX_FUEL];
        fuel_capacity = packet[INDEX_FUEL + 1];
        fuel_low_level = packet[INDEX_FUEL + 2];
        fuel_ltr  = fuel_capacity * (fuel / 100.0);
        outside_temp = packet[INDEX_AMBIENT] - CALIBRATION_TEMPERATURE_OFFSET;
        uint8_t status = packet[INDEX_SPARK];
        if(status & LIMITER_MASK)   rev_limiter = true;//this flag is set if ANY rev limiter or spark cut is active
        if(status & SYNC_MASK)      sync_loss = true; 
        setFlags();
    }
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
            F_CHECK_ENGINE = true;
            F_GENERAL_WARNING = true;
        }
        else{
            F_CHECK_ENGINE = false;
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