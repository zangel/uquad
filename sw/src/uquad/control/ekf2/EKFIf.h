#ifndef UQUAD_CONTROL_EKF2_EKF_IF_H
#define UQUAD_CONTROL_EKF2_EKF_IF_H

#include "../Config.h"
#include "Types.h"
#include "RingBuffer.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    class EKFIf
    {
    public:
        EKFIf();
        virtual ~EKFIf();
        
        // set delta angle imu data
        void setIMUData(uint64_t time_us, uint64_t delta_ang_dt, uint64_t delta_vel_dt, float *delta_ang, float *delta_vel);
        void setMagData(uint64_t time_us, float *data);
        void setBaroData(uint64_t time_us, float *data);
        
        
        virtual bool update();
        
        Quaternionf getAttitude() const;
        Vec3f getPosition() const;
        Vec3f getVelocity() const;
        Vec3f getMagI() const;
        Vec3f getMagB() const;
        
        
    protected:
        virtual bool init(uint64_t time_us);
        
        virtual bool collectIMU(IMUSample &imu) { return true; }
        virtual bool collectBaro(uint64_t time_us, float *data) { return true; }
        
        
    
    protected:
        Parameters _params;		// filter parameters

        static const uint8_t OBS_BUFFER_LENGTH = 10;
        static const uint8_t IMU_BUFFER_LENGTH = 30;
        static const unsigned FILTER_UPDATE_PERRIOD_MS = 10;

        float _dt_imu_avg;
        uint64_t _imu_time_last;
        
        IMUSample _imu_sample_delayed;

        MagSample _mag_sample_delayed;
        BaroSample _baro_sample_delayed;
        GPSSample _gps_sample_delayed;
        RangeSample _range_sample_delayed;
        AirspeedSample _airspeed_sample_delayed;
        FlowSample _flow_sample_delayed;

        OutputSample _output_sample_delayed;
        OutputSample _output_new;
        IMUSample _imu_sample_new;

        uint64_t _imu_ticks;

        bool _imu_updated = false;
        bool _initialised = false;
        bool _vehicle_armed = false;     // vehicle arm status used to turn off functionality used on the ground

        bool _NED_origin_initialised = false;
        bool _gps_speed_valid = false;

        bool _mag_healthy = false;		// computed by mag innovation test
        float _yaw_test_ratio;          // yaw innovation consistency check ratio
        float _mag_test_ratio[3];       // magnetometer XYZ innovation consistency check ratios

        float _vel_pos_test_ratio[6];   // velocity and position innovation consistency check ratios

        RingBuffer<IMUSample, IMU_BUFFER_LENGTH> _imu_buffer;
        RingBuffer<GPSSample, OBS_BUFFER_LENGTH> _gps_buffer;
        RingBuffer<MagSample, OBS_BUFFER_LENGTH> _mag_buffer;
        RingBuffer<BaroSample, OBS_BUFFER_LENGTH> _baro_buffer;
        RingBuffer<RangeSample, OBS_BUFFER_LENGTH> _range_buffer;
        RingBuffer<AirspeedSample, OBS_BUFFER_LENGTH> _airspeed_buffer;
        RingBuffer<FlowSample, OBS_BUFFER_LENGTH> 	_flow_buffer;
        RingBuffer<OutputSample, IMU_BUFFER_LENGTH> _output_buffer;

        uint64_t _time_last_imu;
        uint64_t _time_last_gps;
        uint64_t _time_last_mag;
        uint64_t _time_last_baro;
        uint64_t _time_last_range;
        uint64_t _time_last_airspeed;


        FaultStatus _fault_status;
    };
    

} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_EKF_IF_H