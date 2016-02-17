#ifndef UQUAD_DEVICE_TYPES_H
#define UQUAD_DEVICE_TYPES_H

#define UQUAD_DEVICE_VENDOR_ID          0x3562
#define UQUAD_DEVICE_PRODUCT_ID         0x0001
#define UQUAD_DEVICE_EP_NUM             1


#define UQUAD_DEVICE_PKT_MEASUREMENT    0x00
#define UQUAD_DEVICE_PKT_LEDS_MODE      0x01
#define UQUAD_DEVICE_PKT_MOTORS_PWM     0x02

#define UQUAD_DEVICE_PKT_SIZE           64

#ifndef __XC8
#pragma pack(push,1)
#endif

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} vec3s16_t;

typedef struct
{
    vec3s16_t acceleration;
    vec3s16_t rotation_rate;
    vec3s16_t magnetic_field;
    uint16_t atm_temperature;
    uint32_t atm_pressure;
} measurement_t;

typedef struct
{
    uint8_t led0_mode;
    uint8_t led1_mode;
    uint8_t led2_mode;
    uint8_t led3_mode;
} leds_mode_t;

typedef struct
{
    uint8_t motor0_pwm;
    uint8_t motor1_pwm;
    uint8_t motor2_pwm;
    uint8_t motor3_pwm;
} motors_pwm_t;

typedef union
{
    struct
    {
        uint8_t id;

        union
        {
            measurement_t measurement;
            leds_mode_t leds_mode;
            motors_pwm_t motors_pwm;
        };

    } packet;

    uint8_t raw[UQUAD_DEVICE_PKT_SIZE];
    
} uquad_device_packet_t;

#ifndef __XC8
#pragma pack(pop)
#endif


#endif //UQUAD_DEVICE_TYPES_H