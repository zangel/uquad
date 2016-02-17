#ifndef UQUAD_SENSORS_H
#define UQUAD_SENSORS_H

#include "config.h"
#include "uquad_device_types.h"

#define SENSORS_MPU6K_I2C_ADDR              0xD2

#define SENSORS_MPU6K_SELF_TEST_X           0x0D
#define SENSORS_MPU6K_SELF_TEST_Y           0x0E
#define SENSORS_MPU6K_SELF_TEST_Z           0x0F
#define SENSORS_MPU6K_SELF_TEST_A           0x10
#define SENSORS_MPU6K_SMPLRT_DIV            0x19
#define SENSORS_MPU6K_CONFIG                0x1A
#define SENSORS_MPU6K_GYRO_CONFIG           0x1B
#define SENSORS_MPU6K_ACCEL_CONFIG          0x1C
#define SENSORS_MPU6K_MOT_THR               0x1F
#define SENSORS_MPU6K_FIFO_EN               0x23
#define SENSORS_MPU6K_I2C_MST_CTRL          0x24
#define SENSORS_MPU6K_I2C_SLV0_ADDR         0x25
#define SENSORS_MPU6K_I2C_SLV0_REG          0x26
#define SENSORS_MPU6K_I2C_SLV0_CTRL         0x27
#define SENSORS_MPU6K_I2C_SLV1_ADDR         0x28
#define SENSORS_MPU6K_I2C_SLV1_REG          0x29
#define SENSORS_MPU6K_I2C_SLV1_CTRL         0x2A
#define SENSORS_MPU6K_I2C_SLV2_ADDR         0x2B
#define SENSORS_MPU6K_I2C_SLV2_REG          0x2C
#define SENSORS_MPU6K_I2C_SLV2_CTRL         0x2D
#define SENSORS_MPU6K_I2C_SLV3_ADDR         0x2E
#define SENSORS_MPU6K_I2C_SLV3_REG          0x2F
#define SENSORS_MPU6K_I2C_SLV3_CTRL         0x30
#define SENSORS_MPU6K_I2C_SLV4_ADDR         0x31
#define SENSORS_MPU6K_I2C_SLV4_REG          0x32
#define SENSORS_MPU6K_I2C_SLV4_DO           0x33
#define SENSORS_MPU6K_I2C_SLV4_CTRL         0x34
#define SENSORS_MPU6K_I2C_SLV4_DI           0x35
#define SENSORS_MPU6K_I2C_MST_STATUS        0x36
#define SENSORS_MPU6K_INT_PIN_CFG           0x37
#define SENSORS_MPU6K_INT_ENABLE            0x38
#define SENSORS_MPU6K_INT_STATUS            0x3A
#define SENSORS_MPU6K_RAW_ACCEL             0x3B
#define SENSORS_MPU6K_ACCEL_XOUT_H          0x3B
#define SENSORS_MPU6K_ACCEL_XOUT_L          0x3C
#define SENSORS_MPU6K_ACCEL_YOUT_H          0x3D
#define SENSORS_MPU6K_ACCEL_YOUT_L          0x3E
#define SENSORS_MPU6K_ACCEL_ZOUT_H          0x3F
#define SENSORS_MPU6K_ACCEL_ZOUT_L          0x40
#define SENSORS_MPU6K_TEMP_OUT_H            0x41
#define SENSORS_MPU6K_TEMP_OUT_L            0x42
#define SENSORS_MPU6K_RAW_GYRO              0x43
#define SENSORS_MPU6K_GYRO_XOUT_H           0x43
#define SENSORS_MPU6K_GYRO_XOUT_L           0x44
#define SENSORS_MPU6K_GYRO_YOUT_H           0x45
#define SENSORS_MPU6K_GYRO_YOUT_L           0x46
#define SENSORS_MPU6K_GYRO_ZOUT_H           0x47
#define SENSORS_MPU6K_GYRO_ZOUT_L           0x48
#define SENSORS_MPU6K_EXT_SENS_DATA_00      0x49
#define SENSORS_MPU6K_EXT_SENS_DATA_01      0x4A
#define SENSORS_MPU6K_EXT_SENS_DATA_02      0x4B
#define SENSORS_MPU6K_EXT_SENS_DATA_03      0x4C
#define SENSORS_MPU6K_EXT_SENS_DATA_04      0x4D
#define SENSORS_MPU6K_EXT_SENS_DATA_05      0x4E
#define SENSORS_MPU6K_EXT_SENS_DATA_06      0x4F
#define SENSORS_MPU6K_EXT_SENS_DATA_07      0x50
#define SENSORS_MPU6K_EXT_SENS_DATA_08      0x51
#define SENSORS_MPU6K_EXT_SENS_DATA_11      0x54
#define SENSORS_MPU6K_EXT_SENS_DATA_09      0x52
#define SENSORS_MPU6K_EXT_SENS_DATA_10      0x53
#define SENSORS_MPU6K_EXT_SENS_DATA_12      0x55
#define SENSORS_MPU6K_EXT_SENS_DATA_13      0x56
#define SENSORS_MPU6K_EXT_SENS_DATA_14      0x57
#define SENSORS_MPU6K_EXT_SENS_DATA_15      0x58
#define SENSORS_MPU6K_EXT_SENS_DATA_16      0x59
#define SENSORS_MPU6K_EXT_SENS_DATA_17      0x5A
#define SENSORS_MPU6K_EXT_SENS_DATA_18      0x5B
#define SENSORS_MPU6K_EXT_SENS_DATA_19      0x5C
#define SENSORS_MPU6K_EXT_SENS_DATA_20      0x5D
#define SENSORS_MPU6K_EXT_SENS_DATA_21      0x5E
#define SENSORS_MPU6K_EXT_SENS_DATA_22      0x5F
#define SENSORS_MPU6K_EXT_SENS_DATA_23      0x60
#define SENSORS_MPU6K_I2C_SLV0_DO           0x63
#define SENSORS_MPU6K_I2C_SLV1_DO           0x64
#define SENSORS_MPU6K_I2C_SLV2_DO           0x65
#define SENSORS_MPU6K_I2C_SLV3_DO           0x66
#define SENSORS_MPU6K_I2C_MST_DELAY_CT      0x67
#define SENSORS_MPU6K_SIGNAL_PATH_RES       0x68
#define SENSORS_MPU6K_MOT_DETECT_CTRL       0x69
#define SENSORS_MPU6K_USER_CTRL             0x6A
#define SENSORS_MPU6K_PWR_MGMT_1            0x6B
#define SENSORS_MPU6K_PWR_MGMT_2            0x6C
#define SENSORS_MPU6K_FIFO_COUNTH           0x72
#define SENSORS_MPU6K_FIFO_COUNTL           0x73
#define SENSORS_MPU6K_FIFO_R_W              0x74
#define SENSORS_MPU6K_WHO_AM_I              0x75

#define SENSORS_MPU6K_WHO_AM_I_VALUE        0x68


#define SENSORS_MPL3115A2_I2C_ADDRESS             0xC0

#define SENSORS_MPL3115A2_I2C_PRESSURE_MSB        0x01
#define SENSORS_MPL3115A2_I2C_PRESSURE_CSB        0x02
#define SENSORS_MPL3115A2_I2C_PRESSURE_LSB        0x03
#define SENSORS_MPL3115A2_I2C_TEMP_MSB            0x04
#define SENSORS_MPL3115A2_I2C_TEMP_LSB            0x05
#define SENSORS_MPL3115A2_I2C_WHOAMI              0x0C
#define SENSORS_MPL3115A2_I2C_CTRL_REG1           0x26

#define SENSORS_MPL3115A2_I2C_WHOAMI_VALUE        0xC4


#define SENSORS_AMI304_I2C_ADDRESS                0x1C

#define SENSORS_AMI304_I2C_WHOAMI                 0x0F
#define SENSORS_AMI304_I2C_CNTL1                  0x1B
#define SENSORS_AMI304_I2C_CNTL2                  0x1C
#define SENSORS_AMI304_I2C_CNTL3                  0x1D
#define SENSORS_AMI304_I2C_DATA_X_L               0x10
#define SENSORS_AMI304_I2C_DATA_X_H               0x11
#define SENSORS_AMI304_I2C_DATA_Y_L               0x12
#define SENSORS_AMI304_I2C_DATA_Y_H               0x13
#define SENSORS_AMI304_I2C_DATA_Z_L               0x14
#define SENSORS_AMI304_I2C_DATA_Z_H               0x15

#define SENSORS_AMI304_I2C_WHOAMI_VALUE           0x47



// Compass Register map
/*
#define compass_CRA_REG_M 0x00
#define compass_CRB_REG_M 0x01
#define compass_MR_REG_M  0x02
#define compass_SR_REG_M  0x09
#define compass_IRA_REG_M 0x0A
#define compass_IRB_REG_M 0x0B
#define compass_IRC_REG_M 0x0C
#define compass_OUT_X_H_M 0x03
#define compass_OUT_X_L_M 0x04
#define compass_OUT_Z_H_M 0x05
#define compass_OUT_Z_L_M 0x06
#define compass_OUT_Y_H_M 0x07
#define compass_OUT_Y_L_M 0x08
*/

void sensors_init(void);
void sensors_process(void);
void sensors_isr(void);

void sensors_measurement_start();
void sensors_measurement_stop();
bool sensors_measurement_get(measurement_t *m);


#endif //UQUAD_SENSORS_H