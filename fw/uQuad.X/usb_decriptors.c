#include <usb/usb.h>
#include <usb/usb_device_hid.h>

#include "usb_config.h"
#include "uquad_device_types.h"

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR device_dsc =
{
    0x12,                               // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,              // DEVICE descriptor type
    0x0200,                             // USB Spec Release Number in BCD format
    0x00,                               // Class Code
    0x00,                               // Subclass code
    0x00,                               // Protocol code
    USB_EP0_BUFF_SIZE,                  // Max packet size for EP0, see usb_config.h
    UQUAD_DEVICE_VENDOR_ID,             // Vendor ID, Baldzarika
    UQUAD_DEVICE_PRODUCT_ID,            // Product ID: uQuadDevice HID device
    0x0100,                             // Device release number in BCD format
    0x01,                               // Manufacturer string index
    0x02,                               // Product string index
    0x00,                               // Device serial number string index
    0x01                                // Number of possible configurations
};

/* Configuration 1 Descriptor */
const uint8_t configDescriptor1[] =
{
    /* Configuration Descriptor */
    0x09,                                       // sizeof(USB_CFG_DSC), Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,               // CONFIGURATION descriptor type
    0x20,0x00,                                  // Total length of data for this cfg
    1,                                          // Number of interfaces in this cfg
    1,                                          // Index value of this configuration
    0,                                          // Configuration string index
    _DEFAULT | _SELF,                           // Attributes, see usb_device.h
    50,                                         // Max power consumption (2X mA)

    /* Interface Descriptor */
    0x09,                                       // sizeof(USB_INTF_DSC), size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,                   // INTERFACE descriptor type
    0,                                          // Interface Number
    0,                                          // Alternate Setting Number
    2,                                          // Number of endpoints in this intf
    0xFF,                                       // Class code
    0xFF,                                       // Subclass code
    0xFF,                                       // Protocol code
    0,                                          // Interface string index

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,                    // Endpoint Descriptor
    UQUAD_DEVICE_EP_NUM | _EP_OUT,              // EndpointAddress
    _BULK,                                      // Attributes
    DESC_CONFIG_WORD(UQUAD_DEVICE_PKT_SIZE),    // size
    0x01,                                       // Interval

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,                    // Endpoint Descriptor
    UQUAD_DEVICE_EP_NUM | _EP_IN,               // EndpointAddress
    _BULK,                                      // Attributes
    DESC_CONFIG_WORD(UQUAD_DEVICE_PKT_SIZE),    // size
    0x01                                        // Interval
};

//Language code string descriptor
const struct
{
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[1];
} sd000 =
{
    sizeof(sd000),
    USB_DESCRIPTOR_STRING,
    {
        0x0409
    }
};

//Manufacturer string descriptor
const struct
{
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[10];
} sd001 =
{
    sizeof(sd001),
    USB_DESCRIPTOR_STRING,
    {
        'B','a','l','d','z','a','r','i','k','a'
    }
};

//Product string descriptor
const struct
{
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[12];
} sd002 =
{
    sizeof(sd002),
    USB_DESCRIPTOR_STRING,
    {
        'u','Q','u','a','d', ' ', 'D','e','v','i','c','e'
    }
};

//Array of configuration descriptors
const uint8_t *const USB_CD_Ptr[] =
{
    (const uint8_t *const)&configDescriptor1
};

//Array of string descriptors
const uint8_t *const USB_SD_Ptr[] =
{
    (const uint8_t *const)&sd000,
    (const uint8_t *const)&sd001,
    (const uint8_t *const)&sd002
};
