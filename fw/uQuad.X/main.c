/* 
 * File:   main.c
 * Author: zangel
 *
 * Created on May 10, 2015, 11:21 PM
 */
#include "app.h"

void main(void)
{
    //INTCON = 0x00;

    app_init();

    while(1) app_process();
}

void interrupt interrupt_high(void)
{
    app_isr();
}

void interrupt low_priority interrupt_low(void)
{
}
