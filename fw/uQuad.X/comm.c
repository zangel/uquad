#include "comm.h"
#include "system.h"

#include <usb/usb.h>
#include <usb/usb_device_generic.h>
#include "uquad_device_types.h"
#include "sensors.h"
#include "leds.h"
#include "motors.h"

uquad_device_packet_t comm_out_buffer @ USB_OUT_DATA_BUFFER_ADDRESS;
uquad_device_packet_t comm_in_buffer @ USB_IN_DATA_BUFFER_ADDRESS;


volatile USB_HANDLE comm_out_handle;
volatile USB_HANDLE comm_in_handle;
volatile bool comm_usb_is_configured;

void comm_init(void)
{
    comm_out_handle = 0;
    comm_in_handle = 0;

    USBDeviceInit();
    USBDeviceAttach();
    leds_status_led_mode.comm = 1;
}

void comm_isr(void)
{
    USBDeviceTasks();
}

void comm_process(void)
{
    if(USBGetDeviceState() < CONFIGURED_STATE) return;
    if(USBIsDeviceSuspended()) return;

    if(!USBHandleBusy(comm_out_handle))
    {
        switch(comm_out_buffer.packet.id)
        {
        case UQUAD_DEVICE_PKT_LEDS_MODE:
            leds_led0_mode = comm_out_buffer.packet.leds_mode.led0_mode;
            leds_led1_mode = comm_out_buffer.packet.leds_mode.led1_mode;
            leds_led2_mode = comm_out_buffer.packet.leds_mode.led2_mode;
            leds_led3_mode = comm_out_buffer.packet.leds_mode.led3_mode;
            break;

        case UQUAD_DEVICE_PKT_MOTORS_PWM:
            motors_set_pwm(
                ((uint16_t)comm_out_buffer.packet.motors_pwm.motor0_pwm * MOTORS_MAX_PWM) >> 8,
                ((uint16_t)comm_out_buffer.packet.motors_pwm.motor1_pwm * MOTORS_MAX_PWM) >> 8,
                ((uint16_t)comm_out_buffer.packet.motors_pwm.motor2_pwm * MOTORS_MAX_PWM) >> 8,
                ((uint16_t)comm_out_buffer.packet.motors_pwm.motor3_pwm * MOTORS_MAX_PWM) >> 8
            );
            break;
        }
        comm_out_handle = USBGenRead(UQUAD_DEVICE_EP_NUM, (uint8_t*)&comm_out_buffer, UQUAD_DEVICE_PKT_SIZE);
    }

    
    if(!USBHandleBusy(comm_in_handle))
    {
        if(sensors_measurement_get(&comm_in_buffer.packet.measurement))
        {
            comm_in_buffer.packet.id = UQUAD_DEVICE_PKT_MEASUREMENT;
            comm_in_handle = USBGenWrite(UQUAD_DEVICE_EP_NUM, (uint8_t*)&comm_in_buffer, UQUAD_DEVICE_PKT_SIZE);

        }
    }
}

void comm_usb_configured(void)
{
    comm_in_buffer.packet.id = 0;
    comm_in_handle = 0;

    USBEnableEndpoint(UQUAD_DEVICE_EP_NUM, USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    //Re-arm the OUT endpoint for the next packet
    comm_out_handle = (volatile USB_HANDLE)USBGenRead(UQUAD_DEVICE_EP_NUM,(uint8_t*)&comm_out_buffer, UQUAD_DEVICE_PKT_SIZE);
}

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch((int)event)
    {
    case EVENT_TRANSFER:
        break;

    case EVENT_SOF:
        break;

    case EVENT_SUSPEND:
        break;

    case EVENT_RESUME:
        break;

    case EVENT_CONFIGURED:
        comm_usb_configured();
        break;

    case EVENT_SET_DESCRIPTOR:
        break;

    case EVENT_EP0_REQUEST:
        USBCheckVendorRequest();
        break;

    case EVENT_BUS_ERROR:
        break;

    case EVENT_TRANSFER_TERMINATED:
        break;

    default:
        break;
    }

    return true;
}