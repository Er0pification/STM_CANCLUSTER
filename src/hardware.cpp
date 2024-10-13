#include "hardware.h"
#include "flags.h"
//#include "GyverBME280.h"
enum InputName{
    Undefined1,
    Undefined2,
    Undefined3,
    Undefined4,
    Undefined5,
    Undefined6,
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
    
    inp[Undefined1].PIN = DI7;
    inp[Undefined1].IS_INVERTED= true;
    //inp[Undefined1].FLAG = &F_Undefined1;

    inp[Undefined2].PIN = DI8;
    inp[Undefined2].IS_INVERTED = true;
    //inp[Undefined2].FLAG = &F_Undefined2;

    inp[Undefined3].PIN = DI9;
    inp[Undefined3].IS_INVERTED = true;
    //inp[Undefined3].FLAG = &F_Undefined3;

    inp[Undefined4].PIN = DI10;
    inp[Undefined4].IS_INVERTED = true;
    //inp[Undefined4].FLAG = &F_Undefined4;

    inp[Undefined5].PIN = DI11;
    inp[Undefined5].IS_INVERTED = false;
    //inp[Undefined5].FLAG = &F_Undefined5;

    inp[Undefined6].PIN = DI12;
    inp[Undefined6].IS_INVERTED = false;
    //inp[Undefined6].FLAG  = &F_Undefined6;

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