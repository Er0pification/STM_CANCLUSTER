#include "hardware.h"
#include "flags.h"
//#include "GyverBME280.h"
enum InputName{
    OilPress,
    LowBrakeFluid,
    ParkBrake,
    Door,
    Highbeam,
    Sidelamp,
    LTURN,
    RTURN,
    InputNumber
};
//extern struct Input inp[InputNumber];
Input inp[InputNumber];
//GyverBME280 bmp;
//uint16_t BARO;
uint8_t btn_press;
bool btn_longpress;
uint8_t RAW[8];

void initialize_inputs (void)
{
    
    inp[OilPress].PIN = DI1;
    inp[OilPress].IS_INVERTED= true;
    inp[OilPress].FLAG = &F_OILPRESS;

    inp[LowBrakeFluid].PIN = DI2;
    inp[LowBrakeFluid].IS_INVERTED = true;
    inp[LowBrakeFluid].FLAG = &F_BRAKE_FLUID;

    inp[ParkBrake].PIN = DI3;
    inp[ParkBrake].IS_INVERTED = true;
    inp[ParkBrake].FLAG = &F_BRAKE_PARK;

    inp[Door].PIN = DI4;
    inp[Door].IS_INVERTED = true;
    inp[Door].FLAG = &F_DOOR;

    inp[Highbeam].PIN = DI6;
    inp[Highbeam].IS_INVERTED = false;
    inp[Highbeam].FLAG = &F_HIBEAM;

    inp[Sidelamp].PIN = DI7;
    inp[Sidelamp].IS_INVERTED = false;
    inp[Sidelamp].FLAG  = &F_SIDELAMP;

    inp[LTURN].PIN = DI8;
    inp[LTURN].IS_INVERTED = false;
    inp[LTURN].FLAG = &F_LTURN;

    inp[RTURN].PIN = DI9;
    inp[RTURN].IS_INVERTED = false;
    inp[RTURN].FLAG = &F_RTURN;
    //initializeBaro();
    pinMode(BUTTON, INPUT_PULLUP);
}

void read_inputs (void)
{
    for (int i = 0; i<InputNumber; i++)
    {
        inp[i].STATE = digitalRead(inp[i].PIN);        
        if (inp[i].IS_INVERTED == true) inp[i].STATE = !inp[i].STATE;
        *inp[i].FLAG = inp[i].STATE;
        RAW[i] = inp[i].STATE;
    }
   //BARO =  bmp.readPressure()/100;
   read_button();
}
void getRAW (uint8_t *ARRAY)
{
    for (uint8_t i = 0; i<8; i++)
    {
            ARRAY[i] = RAW[i];
    }
    
}
void read_button (void)
{
    static bool prev_state;
    static uint8_t down_cnt;
    if (!digitalRead(BUTTON))
    {
        down_cnt++;
        prev_state = true;
        if (down_cnt>=20) //ifbutton is pressed fo longer than a second, must be a long press
        {
            btn_longpress = true;
            down_cnt--;
        }
    }
    if (digitalRead(BUTTON) && prev_state == true)
    {
        if(down_cnt<20 && down_cnt>=1) //was pressed for less than 500 ms but more than 100ms
        {
            btn_press++;
        }
        prev_state = false;
        down_cnt = 0;
    }

}

/*void initializeBaro (void)
{
    pinMode(A8, OUTPUT);
    digitalWrite(A8, HIGH);
    delay(50);
    bmp.begin();
}*/