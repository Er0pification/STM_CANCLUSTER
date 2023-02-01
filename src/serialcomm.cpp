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
float PW;
uint16_t RPM;
uint16_t avgF_prev;
uint16_t instF_prev;
float alfa = 0.5;
uint32_t time, time_prev;
float trip;
float odo_tmp;
double fuelCC;
uint8_t odoArr[] = {0,16,32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };
uint8_t odoIndex;

void InitializeSerial(void)
{
    Serial2.begin(115200);
    DbgSerial.begin(115200);
    DbgSerial.println("Serial INITIALIZING");
    ping();
}

char sendCommand(char command)
{
    Serial2.write(command);
    /*char response = serialWait();
    if (response == 0)
    {
        return 0;
    }*/
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
            no_resp = true;
            return 0;            
        }
    }
    no_resp= false;
    return 1;
}

void ping (void)
{
    sendCommand('S');
    if (serialWait())
    {
        char buffer[30];
        int i;
        while(serialWait()){
            buffer[i++] = Serial2.read();
        }
        char msg[i-2];
        strcpy (msg, buffer);
        DbgSerial.println(msg);
    }
    else DbgSerial.println("No ping");
}

void serialRequestData (uint16_t offset, uint16_t length)
{
    sendCommand(COMM_REQUEST_DATA);
    sendCommand(tsCanId);
    sendCommand(0x30);
    sendCommand(lowByte(offset));
    sendCommand(highByte(offset));
    sendCommand(lowByte(length));
    sendCommand(highByte(length));    
    if (!serialWait())
    {
        request_success = false;
        DbgSerial.println("Timeout");
        return;
    }
    length+=2;
    char packet[length]; //response should start with 'r' 0x30
    uint8_t index = 0;
    while (index<length)
    {
        //Read the next available byte in the serial receive buffer
        //if (!serialWait()) return;
        serialWait();
        packet[index++] = Serial2.read();
    }
    //sanity check - packet should begin with 'r' 0x30 and be the correct length
    while (Serial2.available()&& index<100) 
    {
        Serial2.read(); //clear rx buffer
        index++;
    }
    if (packet[0]!='r' || packet[1]!=0x30 || Serial2.available()) 
    {
        data_valid = false;
        DbgSerial.println("Wrong response!");
        DbgSerial.printf("[0] = %c\t[1] = 0x%2X\t L= %d I= %d\n",packet[0],packet[1], length, index);
        //return; 
    }
    else  //if everything seems fine - parse the packet
    {   
        data_valid = true;
        switch (offset)
        {
        case OFF1:{ 
            timeCalc();
            MAP = packet[INDEX_MAP] | (packet[INDEX_MAP+1]<<8);
            clt = packet[INDEX_CLT] - CALIBRATION_TEMPERATURE_OFFSET;
            voltage = packet[INDEX_VOLTAGE];
            rpm = packet[INDEX_RPM] | (packet[INDEX_RPM+1]<<8);
            PW = (packet[INDEX_PW] | (packet[INDEX_PW+1]<<8))/10;
            fuelCalc();}
            break;
        case OFF2:{
            outside_temp = packet[INDEX_AMBIENT] - CALIBRATION_TEMPERATURE_OFFSET;
            oilt = packet[INDEX_OILT] - CALIBRATION_TEMPERATURE_OFFSET;}
            break;
        case OFF3:{
            int temp_speed =  packet[INDEX_VSS] | (packet[INDEX_VSS+1]<<8); //speeds beyond 255 kmh are unlikely
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
            if(status & LIMITER_MASK)   rev_limiter = true; else rev_limiter = false;//this flag is set if ANY rev limiter or spark cut is active
            if(status & SYNC_MASK)      sync_loss = true; else sync_loss = false; }
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
            F_GENERAL_WARNING = true;
            F_LOCK_BLINK = true;
            }
        else{
            F_AIRBAG_BLINK = false;
            F_PASS_AIRBAG_BLINK = false;
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
    float inc = ((float)speed/3.6)*((float)time/1000); //speed in m/s * time in s
    data.tripMeter+=inc;
    odo_tmp += inc;
    if (odo_tmp> 160) 
    {
        if (odoIndex>=15) odoIndex = 0;
        else odoIndex++;
        odo_cnt = odoArr[odoIndex];
        odo_tmp -= 160;
    }

}
void timeCalc (void)
{
    time = millis() - time_prev;
    time_prev = millis();
}
void fuelCalc (void)
{
    
    uint8_t injN=6;
    uint8_t nSquirts = 2;
    uint8_t div = 2;
    uint16_t baseFlow = 296;
    uint16_t basePressure = 270;

    float deadtime = 1;

    float fuelPres = basePressure + (MAP-100);//kpa
    float flowCor = baseFlow * sqrt(fuelPres/basePressure); //cc/min 
    double squirtsPerEvent = (rpm*time/60000) * (injN * nSquirts / div);
    //                        RPS       Ms to S             3 injections
    //                      {Revolutions per event}           per revolution
    uint32_t squirtsPerHr =    rpm * 60 * injN * nSquirts / div;
    //                          RPH
    
    double fuelPerSquirt = ((PW-deadtime)/60000) * flowCor;
    //                      (ms         to min) * cc/min = cc
    //data.CumulativeFuel += squirtsPerEvent * fuelPerSquirt;
    fuelCC += squirtsPerEvent * fuelPerSquirt;
    if (fuelCC>= 100) 
    {
        data.CumulativeFuel ++;
        fuelCC-=100;
    }
    double lhrCons = (squirtsPerHr * fuelPerSquirt) / 100000.0; //TODO  - find out where excessive 00 came from - MUST BE 1000!
    instF = (int)(lhrCons*10);
    instF = alfa*instF + (1-alfa)* instF_prev;
    instF_prev = instF;
    double avgTemp = (data.CumulativeFuel/100) / (data.tripMeter/100000);
    avgF = int(avgTemp);
    DbgSerial.printf("/nFuel total - %d, avg - %d, sqirts - %d", (int)data.CumulativeFuel, avgF, (int)squirtsPerEvent);
}

void tripReset (void)
{
    data.tripMeter = 0;
    data.CumulativeFuel = 0;
}