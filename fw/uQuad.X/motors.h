#ifndef UQUAD_MOTORS_H
#define UQUAD_MOTORS_H

#include "config.h"


#define MOTORS_MAX_PWM  94


void motors_init(void);
void motors_process(void);
void motors_isr(void);
void motors_set_pwm(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3);


#endif //UQUAD_MOTORS_H

