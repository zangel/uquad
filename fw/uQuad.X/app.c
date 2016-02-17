#include "app.h"
#include "leds.h"
#include "motors.h"
#include "sensors.h"
#include "comm.h"


void app_init()
{
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    ADCON1 = 0x0F;
    TRISBbits.TRISB4 = 1;

    T3CONbits.TMR3ON = 0;
    T3CONbits.RD16 = 1;
    T3CONbits.T3CKPS = 0b11;
    T3CONbits.TMR3CS = 0;

    PIR2bits.TMR3IF = 0;
    PIE2bits.TMR3IE = 0;
    TMR3 = 0;
    T3CONbits.TMR3ON = 1;
    
    leds_init();
    motors_init();
    sensors_init();
    sensors_measurement_start();
    comm_init();
}

void app_process()
{
    leds_process();
    motors_process();
    sensors_process();
    comm_process();
}

void app_isr()
{
    leds_isr();
    motors_isr();
    sensors_isr();
    comm_isr();
}