#include "leds.h"

leds_status_led_mode_t leds_status_led_mode;
uint8_t leds_led0_mode;
uint8_t leds_led1_mode;
uint8_t leds_led2_mode;
uint8_t leds_led3_mode;

static uint8_t leds_counter;

#define LEDS_STATUS_LED_DIR     TRISEbits.RE0
#define LEDS_STATUS_LED_BIT     LATEbits.LATE0

#define LEDS_LED0_DIR           TRISDbits.TRISD3
#define LEDS_LED1_DIR           TRISDbits.TRISD5
#define LEDS_LED2_DIR           TRISDbits.TRISD7
#define LEDS_LED3_DIR           TRISDbits.TRISD1

#define LEDS_LED0_BIT           LATDbits.LATD3
#define LEDS_LED1_BIT           LATDbits.LATD5
#define LEDS_LED2_BIT           LATDbits.LATD7
#define LEDS_LED3_BIT           LATDbits.LATD1

static const uint32_t leds_status_patterns[LEDS_MODE_COUNT] =
{
    /*    |   |       |       |       |    */
    {   0b00000111111001111110011111100000 },
    {   0b00000111111001111110000110000000 },
    {   0b00000111111000011000011111100000 },
    {   0b00000111111000011000000110000000 },
    {   0b00000001100001111110011111100000 },
    {   0b00000001100001111110000110000000 },
    {   0b00000001100000011000011111100000 },
    {   0b00000001100000011000000110000000 },
    {   0b11111111111111111111111111111111 }
};

static const uint32_t leds_quad_patterns[LEDS_MODE_COUNT] =
{
    /*    |       |       |       |       |    */
    {   0b00000000000000000000000000000000 },
    {   0b00001111000011110000111100001111 },
    {   0b11110000111100001111000011110000 },
    {   0b00000000111111110000000011111111 },
    {   0b11111111000000001111111100000000 },
    {   0b00000000000000001111111111111111 },
    {   0b11111111111111110000000000000000 },
    {   0b00000000000000001111111111111111 },
    {   0b11000000110000001100000011000000 },
    {   0b00110000001100000011000000110000 },
    {   0b00001100000011000000110000001100 },
    {   0b00000011000000110000001100000011 },
    {   0b11111111111111111111111111111111 }
};


void leds_init(void)
{
    leds_status_led_mode.mode = LEDS_MODE_0;

    leds_led0_mode = LEDS_MODE_BOOT_0;
    leds_led1_mode = LEDS_MODE_BOOT_1;
    leds_led2_mode = LEDS_MODE_BOOT_2;
    leds_led3_mode = LEDS_MODE_BOOT_3;


    leds_counter = 0;
    
    LEDS_STATUS_LED_DIR = 0;
    LEDS_STATUS_LED_BIT = 0;

    LEDS_LED0_DIR = 0;
    LEDS_LED1_DIR = 0;
    LEDS_LED2_DIR = 0;
    LEDS_LED3_DIR = 0;

    LEDS_LED0_BIT = 0;
    LEDS_LED1_BIT = 0;
    LEDS_LED2_BIT = 0;
    LEDS_LED3_BIT = 0;
    
    PIE1bits.TMR1IE = 0;

    T1CONbits.TMR1ON = 0;
    T1CONbits.TMR1CS = 0;
    T1CONbits.T1CKPS = 0b11;
    T1CONbits.RD16 = 1;

    TMR1 = 0;
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;
    T1CONbits.TMR1ON = 1;
}

void leds_process(void)
{
    
}

void leds_isr(void)
{
    uint32_t pat_flag = 1;
    if(PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;

        leds_counter++;

        pat_flag <<= (leds_counter & 0b00011111);

        if(leds_status_patterns[leds_status_led_mode.mode] & pat_flag)
            LEDS_STATUS_LED_BIT = 1;
        else
            LEDS_STATUS_LED_BIT = 0;

        if(leds_quad_patterns[leds_led0_mode] & pat_flag)
            LEDS_LED0_BIT = 1;
        else
            LEDS_LED0_BIT = 0;

        if(leds_quad_patterns[leds_led1_mode] & pat_flag)
            LEDS_LED1_BIT = 1;
        else
            LEDS_LED1_BIT = 0;

        if(leds_quad_patterns[leds_led2_mode] & pat_flag)
            LEDS_LED2_BIT = 1;
        else
            LEDS_LED2_BIT = 0;

        if(leds_quad_patterns[leds_led3_mode] & pat_flag)
            LEDS_LED3_BIT = 1;
        else
            LEDS_LED3_BIT = 0;
    }
}