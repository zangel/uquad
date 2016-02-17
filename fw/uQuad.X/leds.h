#ifndef UQUAD_LEDS_H
#define UQUAD_LEDS_H

#include "config.h"

enum
{
    LEDS_MODE_0 = 0,
    LEDS_MODE_1,
    LEDS_MODE_2,
    LEDS_MODE_3,
    LEDS_MODE_4,
    LEDS_MODE_5,
    LEDS_MODE_6,
    LEDS_MODE_7,
    LEDS_MODE_BOOT_0,
    LEDS_MODE_BOOT_1,
    LEDS_MODE_BOOT_2,
    LEDS_MODE_BOOT_3,
    LEDS_MODE_ON,
    LEDS_MODE_COUNT
};


extern uint8_t leds_led0_mode;
extern uint8_t leds_led1_mode;
extern uint8_t leds_led2_mode;
extern uint8_t leds_led3_mode;

typedef union
{
    struct
    {
        unsigned motors     :1;
        unsigned sensors    :1;
        unsigned comm       :1;
    };

    uint8_t mode;

} leds_status_led_mode_t;

extern leds_status_led_mode_t leds_status_led_mode;

void leds_init(void);
void leds_process(void);
void leds_isr(void);


#endif //UQUAD_LEDS_H