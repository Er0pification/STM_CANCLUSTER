#include "hardware.h"
#include "flags.h"
#ifdef HASLCD
    #include "lcd.h"
#endif


enum InputName{
    Undefined1,
    Undefined2,
    Undefined3,
    Undefined4,
    Undefined5,
    Undefined6,
    InputNumber
};

enum OutputName{
    LC1,
    LC2,
    LC3,
    LC4,
    LC5,
    HC1,
    HC2,
    HC3,
    HC4,
    OutputNumber
};
//extern struct Input inp[InputNumber];
Input inp[InputNumber];
Output out[OutputNumber];
muxUnion_t muxOut;

uint8_t btn_press;
bool btn_longpress;
uint8_t RAW[8];
void initialize_hardware (void)
{
    initialize_inputs();
    initialize_outputs();
}
void initialize_inputs (void)
{
     bool Unused;
    inp[Undefined1].PIN = DI7;
    inp[Undefined1].IS_INVERTED= true;
    inp[Undefined1].FLAG = &Unused;

    inp[Undefined2].PIN = DI8;
    inp[Undefined2].IS_INVERTED = true;
    inp[Undefined2].FLAG = &Unused;

    inp[Undefined3].PIN = DI9;
    inp[Undefined3].IS_INVERTED = true;
    inp[Undefined3].FLAG = &Unused;

    inp[Undefined4].PIN = DI10;
    inp[Undefined4].IS_INVERTED = true;
    inp[Undefined4].FLAG = &Unused;

    inp[Undefined5].PIN = DI11;
    inp[Undefined5].IS_INVERTED = false;
    inp[Undefined5].FLAG = &Unused;

    inp[Undefined6].PIN = DI12;
    inp[Undefined6].IS_INVERTED = false;
    inp[Undefined6].FLAG  = &Unused;

    //initializeBaro();
    pinMode(BUTTON, INPUT_PULLUP);
}

void initialize_outputs (void)
{
    pinMode(L1,OUTPUT);
    pinMode(L2,OUTPUT);
    pinMode(L3,OUTPUT);
    pinMode(L4,OUTPUT);
    pinMode(L5,OUTPUT);
    pinMode(LS1,OUTPUT);
    pinMode(LS2,OUTPUT);
    pinMode(LS3,OUTPUT);
    pinMode(LS4,OUTPUT);

    pinMode(SHIFT_DATA, OUTPUT);
    pinMode(SHIFT_CLOCK, OUTPUT);
    pinMode(SHIFT_LATCH, OUTPUT);

    out[LC1].PIN = L1;
    out[LC1].IS_INVERTED = false;
    out[LC1].IS_PWM = false;
    //out[LC1].PW = 0;
    //out[LC1].FLAG = &F_UNDEFINED;
    out[LC2].PIN = L2;
    out[LC2].IS_INVERTED = false;
    out[LC2].IS_PWM = false;
    //out[LC2].PW = 0;
    //out[LC2].FLAG = &F_UNDEFINED;
    out[LC3].PIN = L3;
    out[LC3].IS_INVERTED = false;
    out[LC3].IS_PWM = false;
    //out[LC3].PW = 0;
    //out[LC3].FLAG = &F_UNDEFINED;
    out[LC4].PIN = L4;
    out[LC4].IS_INVERTED = false;
    out[LC4].IS_PWM = false;
    //out[LC4].PW = 0;
    //out[LC4].FLAG = &F_UNDEFINED;
    out[LC5].PIN = L5;
    out[LC5].IS_INVERTED = false;
    out[LC5].IS_PWM = false;
    //out[LC5].PW = 0;
    //out[LC5].FLAG = &F_UNDEFINED;

    out[HC1].PIN = LS1;
    out[HC1].IS_INVERTED = false;
    out[HC1].IS_PWM = true;
    out[HC1].PW = 200;
    //out[HC1].FLAG = &F_UNDEFINED;
    out[HC2].PIN = LS2;
    out[HC2].IS_INVERTED = false;
    out[HC2].IS_PWM = false;
    //out[HC2].PW = 0;
    //out[HC2].FLAG = &F_UNDEFINED;
    out[HC3].PIN = LS3;
    out[HC3].IS_INVERTED = false;
    out[HC3].IS_PWM = false;
    //out[HC3].PW = 0;
    //out[HC3].FLAG = &F_UNDEFINED;
    out[HC4].PIN = LS4;
    out[HC4].IS_INVERTED = false;
    out[HC4].IS_PWM = false;
    //out[HC4].PW = 0;
    //out[HC4].FLAG = &F_UNDEFINED;
    
    analogWriteFrequency(10);
    
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
 
   //read_button();
}

void set_outputs (void)
{
    bool state;
    uint8_t pw;
    for (int i = 0; i<OutputNumber; i++)
    {
        if (out[i].IS_PWM)
        {
            if (digitalPinHasPWM(out[i].PIN))
            {
                pw = out[i].PW;
                if(out[i].IS_INVERTED) pw = 255 - pw;
                analogWrite(out[i].PIN, pw);
            }
            else
            {
                 #ifdef HASLCD
                    lcd_terminal("PIN has no pwm!",0);
                 #endif
                
            }
        }
        else
        {
            state = out[i].FLAG;     
            if (out[i].IS_INVERTED == true) state = !state;
            digitalWrite(out[i].PIN, state);
        }
    }
    get_values_for_outputs();
    digitalWrite(SHIFT_LATCH, LOW);
    shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, muxOut.byte);
    digitalWrite(SHIFT_LATCH, HIGH);
}

void get_values_for_outputs (void){
    muxOut.out13 = F_FAN1;
    muxOut.out12 = F_FAN2;
    //muxOut.out11 = &F_UNDEFINED;
    //muxOut.out10 = &F_UNDEFINED;
    //muxOut.out9 = &F_UNDEFINED;
    //muxOut.out8 = &F_UNDEFINED;
    //muxOut.out7 = &F_UNDEFINED;
    //muxOut.out6 = &F_UNDEFINED;
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