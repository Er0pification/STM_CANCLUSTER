#ifndef HARDWARE_H
#define HARDWARE_H
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
struct Input {
    int PIN;
    bool IS_INVERTED;
    bool STATE;
    bool *FLAG;
};
void initialize_inputs(void);
void read_inputs(void);
#endif