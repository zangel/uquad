#include "motors.h"
#include "leds.h"

static volatile uint8_t motors_pwm[4];
static volatile uint8_t motors_pwm_scale[4];
static volatile uint8_t motors_pwm_on_bits;


static uint8_t motors_current;
static uint8_t motors_current_off_duration;
static uint8_t motors_next_frame_wait_count;


#define MOTORS_1MS_DURATION             MOTORS_MAX_PWM
#define MOTORS_0_BIT                    LATDbits.LATD0
#define MOTORS_1_BIT                    LATDbits.LATD2
#define MOTORS_2_BIT                    LATDbits.LATD4
#define MOTORS_3_BIT                    LATDbits.LATD6
#define MOTORS_REG                      LATD

#define MOTORS_0_DIR                    TRISDbits.TRISD0
#define MOTORS_1_DIR                    TRISDbits.TRISD2
#define MOTORS_2_DIR                    TRISDbits.TRISD4
#define MOTORS_3_DIR                    TRISDbits.TRISD6

#define MOTORS_COUNT                    4

/*
#define MOTORS_STATE_CONFIG_HIGH_DURATION       4500000 //4 sec
#define MOTORS_STATE_CONFIG_LOW_DURATION        1500000 //4 sec
#define MOTORS_STATE_ACTIVE_OP_THRESHOLD        80
#define MOTORS_STATE_ACTIVE_CD_SCALE            128     //0.5f
#define MOTORS_STATE_ACTIVE_OP_DURATION         4500000 //3 sec
#define MOTORS_STATE_ACTIVE_CD_DURATION         6000000 //4 sec

#define MOTORS_PROCESS_SKIP_COUNT               0x7F


typedef enum
{
    MOTORS_STATE_ACTIVE=0,
    MOTORS_STATE_ACTIVE_COOLDOWN,
    MOTORS_STATE_CONFIG_WAIT,
    MOTORS_STATE_CONFIG_HIGH,
    MOTORS_STATE_CONFIG_LOW
} motors_state_t;

static motors_state_t motors_state;
static uint32_t motors_state_timer;
static uint16_t motors_state_timer_last;
static bool motors_op;
*/

void motors_init(void)
{
    //PORTEbits.RDPU = 1;
    MOTORS_0_DIR = 0;
    MOTORS_1_DIR = 0;
    MOTORS_2_DIR = 0;
    MOTORS_3_DIR = 0;

    MOTORS_0_BIT = 0;
    MOTORS_1_BIT = 0;
    MOTORS_2_BIT = 0;
    MOTORS_3_BIT = 0;

    motors_current = 0;
    motors_current_off_duration = 0;
    motors_next_frame_wait_count = 0;

    motors_pwm_scale[0] = 255;
    motors_pwm_scale[1] = 255;
    motors_pwm_scale[2] = 255;
    motors_pwm_scale[3] = 255;
    motors_pwm_on_bits = 0x55;

    motors_set_pwm(0,0,0,0);

    //motors_state = MOTORS_STATE_ACTIVE;
    //motors_state_timer = 0;

    leds_status_led_mode.motors = 1;
    
    //motors_state_timer = 0;
    //motors_state_timer_last = TMR3;
    //motors_op = false;

    T0CONbits.TMR0ON = 0;
    T0CONbits.T08BIT = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.T0SE = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS = 0b110;

    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    TMR0L = 0;
    T0CONbits.TMR0ON = 1;
}

void motors_process(void)
{
    /*
    static uint8_t skip_counter = MOTORS_PROCESS_SKIP_COUNT;
    uint32_t state_time_step;
    bool motors_op_now;
    uint16_t now = TMR3;

    skip_counter++;
    if(skip_counter) return;
    skip_counter = MOTORS_PROCESS_SKIP_COUNT;
            
    state_time_step = now < motors_state_timer_last?
        (uint32_t)now + (0xFFFF - motors_state_timer_last):
        now - motors_state_timer_last;
    
    motors_state_timer += state_time_step;
    motors_state_timer_last = now;

    switch(motors_state)
    {
    case MOTORS_STATE_ACTIVE:
        motors_op_now =
                motors_motor0_pwm > MOTORS_STATE_ACTIVE_OP_THRESHOLD ||
                motors_motor1_pwm > MOTORS_STATE_ACTIVE_OP_THRESHOLD ||
                motors_motor2_pwm > MOTORS_STATE_ACTIVE_OP_THRESHOLD ||
                motors_motor3_pwm > MOTORS_STATE_ACTIVE_OP_THRESHOLD;

        if(!motors_op && motors_op_now) motors_state_timer = 0;

        motors_op = motors_op_now;

        if(motors_op)
        {
            if(motors_state_timer > MOTORS_STATE_ACTIVE_OP_DURATION)
            {
                motors_actual_pwm[0] = ((uint16_t)motors_motor0_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
                motors_actual_pwm[1] = ((uint16_t)motors_motor1_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
                motors_actual_pwm[2] = ((uint16_t)motors_motor2_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
                motors_actual_pwm[3] = ((uint16_t)motors_motor3_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
                motors_state = MOTORS_STATE_ACTIVE_COOLDOWN;
                motors_state_timer = 0;
            }
            else
            {
                motors_actual_pwm[0] = motors_motor0_pwm;
                motors_actual_pwm[1] = motors_motor1_pwm;
                motors_actual_pwm[2] = motors_motor2_pwm;
                motors_actual_pwm[3] = motors_motor3_pwm;
            }
        }
        else
        {
            motors_actual_pwm[0] = motors_motor0_pwm;
            motors_actual_pwm[1] = motors_motor1_pwm;
            motors_actual_pwm[2] = motors_motor2_pwm;
            motors_actual_pwm[3] = motors_motor3_pwm;

        }
        leds_status_led_mode.motors = 1;
        break;

    case MOTORS_STATE_ACTIVE_COOLDOWN:
        if(motors_state_timer > MOTORS_STATE_ACTIVE_CD_DURATION)
        {
            motors_actual_pwm[0] = motors_motor0_pwm;
            motors_actual_pwm[1] = motors_motor1_pwm;
            motors_actual_pwm[2] = motors_motor2_pwm;
            motors_actual_pwm[3] = motors_motor3_pwm;
            motors_state = MOTORS_STATE_ACTIVE;
            motors_op = false;
            motors_state_timer = 0;
        }
        else
        {
            motors_actual_pwm[0] = ((uint16_t)motors_motor0_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
            motors_actual_pwm[1] = ((uint16_t)motors_motor1_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
            motors_actual_pwm[2] = ((uint16_t)motors_motor2_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
            motors_actual_pwm[3] = ((uint16_t)motors_motor3_pwm * MOTORS_STATE_ACTIVE_CD_SCALE) >> 8;
        }
        break;

    case MOTORS_STATE_CONFIG_HIGH:
        if(motors_state_timer < MOTORS_STATE_CONFIG_HIGH_DURATION)
        {
            motors_actual_pwm[0] = MOTORS_MAX_PWM;
            motors_actual_pwm[1] = MOTORS_MAX_PWM;
            motors_actual_pwm[2] = MOTORS_MAX_PWM;
            motors_actual_pwm[3] = MOTORS_MAX_PWM;
        }
        else
        {
            motors_actual_pwm[0] = 0;
            motors_actual_pwm[1] = 0;
            motors_actual_pwm[2] = 0;
            motors_actual_pwm[3] = 0;
            motors_state = MOTORS_STATE_CONFIG_LOW;
            motors_state_timer = 0;
        }
        leds_status_led_mode.motors = 0;
        break;

    case MOTORS_STATE_CONFIG_LOW:
        if(motors_state_timer < MOTORS_STATE_CONFIG_LOW_DURATION)
        {
            motors_actual_pwm[0] = 0;
            motors_actual_pwm[1] = 0;
            motors_actual_pwm[2] = 0;
            motors_actual_pwm[3] = 0;
        }
        else
        {
            motors_actual_pwm[0] = motors_motor0_pwm;
            motors_actual_pwm[1] = motors_motor1_pwm;
            motors_actual_pwm[2] = motors_motor2_pwm;
            motors_actual_pwm[3] = motors_motor3_pwm;
            motors_state = MOTORS_STATE_ACTIVE;
            motors_state_timer = 0;
        }
        break;
    }
    */
}

void motors_isr(void)
{
    if(INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        {
            uint8_t motor_pwm_bit = (1 << (motors_current << 1));

            if(motors_current_off_duration)
            {
                MOTORS_REG &= ~motor_pwm_bit;

                TMR0L = motors_current_off_duration;
                motors_current_off_duration = 0;

                motors_current ++;
                motors_current &= 0x03;
            }
            else
            {
                MOTORS_REG |= (motor_pwm_bit & motors_pwm_on_bits);

                motors_current_off_duration = motors_pwm[motors_current];
                TMR0L = (255 - MOTORS_1MS_DURATION) - motors_current_off_duration;
                motors_current_off_duration = 255 - (MOTORS_1MS_DURATION - motors_current_off_duration);
            }
        }
    }
}

void motors_set_pwm(uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3)
{
    if(m0 > 0)
        motors_pwm_on_bits |=  0x01;
    else
        motors_pwm_on_bits &= ~0x01;
    motors_pwm[0] = m0;

    if(m1 > 0)
        motors_pwm_on_bits |=  0x04;
    else
        motors_pwm_on_bits &= ~0x04;
    motors_pwm[1] = m1;

    if(m2 > 0)
        motors_pwm_on_bits |=  0x10;
    else
        motors_pwm_on_bits &= ~0x10;
    motors_pwm[2] = m2;

    if(m3 > 0)
        motors_pwm_on_bits |=  0x40;
    else
        motors_pwm_on_bits &= ~0x40;
    motors_pwm[3] = m3;
}