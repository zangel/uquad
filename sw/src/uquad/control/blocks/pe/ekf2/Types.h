#ifndef UQUAD_CONTROL_EKF2_TYPES_H
#define UQUAD_CONTROL_EKF2_TYPES_H

#include "../Config.h"
#include "../../base/Time.h"
#include "matrix/math.hpp"

namespace uquad
{
namespace control
{
namespace ekf2
{
    typedef matrix::Vector2<float> Vector2f;
    typedef matrix::Vector3<float> Vector3f;
    typedef matrix::Quaternion<float> Quaternion;
    typedef matrix::Matrix<float, 3, 3> Matrix3f;

    
    struct TimeStamped
    {
        uint64_t time_us;
    };
    
    struct GPSMessage : TimeStamped
    {
        float lat;                      // Latitude in degrees
        float lon;                      // Longitude in degrees
        float alt;                      // Altitude in meters (millimeters) above MSL
        uint8_t fix_type;               // 0-1: no fix, 2: 2D fix, 3: 3D fix, 4: RTCM code differential, 5: Real-Time
        float eph;                      // GPS horizontal position accuracy in m
        float epv;                      // GPS vertical position accuracy in m
        float sacc;                     // GPS speed accuracy in m/s
        uint64_t time_usec_vel;         // Timestamp for velocity informations
        float vel_m_s;                  // GPS ground speed (m/s)
        float vel_ned[3];               // GPS ground speed NED
        bool vel_ned_valid;             // GPS ground speed is valid
        uint8_t nsats;                  // number of satellites used
        float gdop;                     // geometric dilution of precision
    };
    
    struct OutputSample : TimeStamped
    {
        Quaternion quat_nominal;
        Vector3f vel;
        Vector3f pos;
        Vector3f mag_I;
        Vector3f mag_B;
    };
    
    struct IMUSample : TimeStamped
    {
        Vector3f delta_ang;
        Vector3f delta_vel;
        float delta_ang_dt;
        float delta_vel_dt;
    };
    
    struct GPSSample : TimeStamped
    {
        Vector2f pos;
        float hgt;
        Vector3f vel;
    };
    
    struct MagSample : TimeStamped
    {
        Vector3f mag;
    };
    
    struct BaroSample : TimeStamped
    {
        float hgt;
    };
    
    struct RangeSample : TimeStamped
    {
        float rng;
    };
    
    struct AirspeedSample : TimeStamped
    {
        float airspeed;
    };
    
    struct FlowSample : TimeStamped
    {
        Vector2f flowRadXY;
        Vector2f flowRadXYcomp;

    };
    
    struct Parameters
    {
        float mag_delay_ms = 0.0f;
        float baro_delay_ms = 0.0f;
        float gps_delay_ms = 200.0f;
        float airspeed_delay_ms = 200.0f;
        
        // input noise
        float gyro_noise = 0.001f;
        float accel_noise = 0.1f;
        
        // process noise
        float gyro_bias_p_noise = 1e-5f;
        float accel_bias_p_noise = 1e-3f;
        float gyro_scale_p_noise = 1e-4f;
        float mag_p_noise = 1e-2f;
        float wind_vel_p_noise = 0.05f;
        
        float gps_vel_noise = 0.05f;
        float gps_pos_noise = 1.0f;
        float baro_noise = 0.1f;
        float baro_innov_gate = 5.0f;       // barometric height innovation consistency gate size in standard deviations
        float posNE_innov_gate = 5.0f;      // GPS horizontal position innovation consistency gate size in standard deviations
        float vel_innov_gate = 3.0f;        // GPS velocity innovation consistency gate size in standard deviations
        
        float mag_heading_noise = 3e-2f;	// measurement noise used for simple heading fusion
        float mag_declination_deg = 0.0f;	// magnetic declination in degrees
        float heading_innov_gate = 3.0f;	// heading fusion innovation consistency gate size in standard deviations
        float mag_innov_gate = 3.0f;        // magnetometer fusion innovation consistency gate size in standard deviations
        
        // these parameters control the strictness of GPS quality checks used to determine uf the GPS is
        // good enough to set a local origin and commence aiding
        int gps_check_mask = 21;    // bitmask used to control which GPS quality checks are used
        float req_hacc = 5.0f;      // maximum acceptable horizontal position error
        float req_vacc = 8.0f;      // maximum acceptable vertical position error
        float req_sacc = 1.0f;      // maximum acceptable speed error
        int req_nsats = 6;          // minimum acceptable satellite count
        float req_gdop = 2.0f;      // maximum acceptable geometric dilution of precision
        float req_hdrift = 0.3f;    // maximum acceptable horizontal drift speed
        float req_vdrift = 0.5f;    // maximum acceptable vertical drift speed
    };
    
    struct StateSample : TimeStamped
    {
        Vector3f ang_error;
        Vector3f vel;
        Vector3f pos;
        Vector3f gyro_bias;
        Vector3f gyro_scale;
        float accel_z_bias;
        Vector3f mag_I;
        Vector3f mag_B;
        Vector3f wind_vel;
        Quaternion quat_nominal;
    };
    
    struct FaultStatus
    {
        bool bad_mag_x: 1;
        bool bad_mag_y: 1;
        bool bad_mag_z: 1;
        bool bad_airspeed: 1;
        bool bad_sideslip: 1;
    };
    
    // publish the status of various GPS quality checks
    union GPSCheckFailStatus
    {
        struct
        {
            uint16_t fix    : 1; // 0 - true if the fix type is insufficient (no 3D solution)
            uint16_t nsats  : 1; // 1 - true if number of satellites used is insufficient
            uint16_t gdop   : 1; // 2 - true if geometric dilution of precision is insufficient
            uint16_t hacc   : 1; // 3 - true if reported horizontal accuracy is insufficient
            uint16_t vacc   : 1; // 4 - true if reported vertical accuracy is insufficient
            uint16_t sacc   : 1; // 5 - true if reported speed accuracy is insufficient
            uint16_t hdrift : 1; // 6 - true if horizontal drift is excessive (can only be used when stationary on ground)
            uint16_t vdrift : 1; // 7 - true if vertical drift is excessive (can only be used when stationary on ground)
            uint16_t hspeed : 1; // 8 - true if horizontal speed is excessive (can only be used when stationary on ground)
            uint16_t vspeed : 1; // 9 - true if vertical speed error is excessive
        } flags;
        uint16_t value;
    };
    
    // bitmask containing filter control status
    union FilterControlStatus
    {
        struct
        {
            uint8_t angle_align : 1; // 0 - true if the filter angular alignment is complete
            uint8_t gps         : 1; // 1 - true if GPS measurements are being fused
            uint8_t opt_flow    : 1; // 2 - true if optical flow measurements are being fused
            uint8_t mag_hdg     : 1; // 3 - true if a simple magnetic heading is being fused
            uint8_t mag_3D      : 1; // 4 - true if 3-axis magnetometer measurement are being fused
            uint8_t mag_dec     : 1; // 5 - true if synthetic magnetic declination measurements are being fused
            uint8_t in_air      : 1; // 6 - true when the vehicle is airborne
            uint8_t armed       : 1; // 7 - true when the vehicle motors are armed
        } flags;
        uint16_t value;
    };

} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_TYPES_H
