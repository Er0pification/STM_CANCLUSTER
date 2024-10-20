#ifndef HARDWARE_H
#define HARDWARE_H

#include<arduino.h>
#define DI1 PB7
#define DI2 PB6
#define DI3 PB5
#define DI4 PB4
#define DI5 PB3
#define DI6 PA15
#define DI7 PA10
#define DI8 PA9
#define DI9 PA8
#define DI10 PB14
#define DI11 PB11
#define DI12 PB10
#define BUTTON PB15
#define L1 PA5
#define L2 PA4
#define L3 PC15
#define L4 PC14
#define L5 PC13
#define LS1 PB1
#define LS2 PB0
#define LS3 PA7
#define LS4 PA6
#define SHIFT_DATA PB15
#define SHIFT_CLOCK PB13
#define SHIFT_LATCH PB12
struct Input {
    int PIN;
    bool IS_INVERTED;
    bool STATE;
    bool *FLAG;
};
struct Output {
    int PIN;
    bool IS_INVERTED;
    bool *FLAG;
    bool IS_PWM;
    uint8_t PW;

};
typedef union{
    uint8_t byte;
    struct{
        bool out13 : 1;
        bool out12 : 1;
        bool out11 : 1;
        bool out10 : 1;
        bool out9 : 1;
        bool out8 : 1;
        bool out7 : 1;
        bool out6 : 1;
    };
} muxUnion_t;
extern  muxUnion_t muxOut;





extern uint8_t btn_press;
extern bool btn_longpress;
void initialize_hardware(void);
void initialize_inputs(void);
void initialize_outputs(void);
void read_inputs(void);
void set_flags(void);
void initializeBaro(void);
void read_button(void);
void getRAW (uint8_t*);
void set_outputs (void);
void get_values_for_outputs (void);
#endif