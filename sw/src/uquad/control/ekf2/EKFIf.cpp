#include "EKFIf.h"
#include "../../base/Error.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    EKFIf::EKFIf()
    {
    }
    
    EKFIf::~EKFIf()
    {
    }
    
    void EKFIf::setIMUData(uint64_t time_us, uint64_t delta_ang_dt, uint64_t delta_vel_dt, float *delta_ang, float *delta_vel)
    {
        if (!_initialised) {
            init(time_us);
            _initialised = true;
        }

        float dt = (float)(time_us - _time_last_imu) / 1000 / 1000;
        dt = std::max(dt, 1.0e-4f);
        dt = std::min(dt, 0.02f);

        _time_last_imu = time_us;

        if (_time_last_imu > 0) {
            _dt_imu_avg = 0.8f * _dt_imu_avg + 0.2f * dt;
        }

        // copy data
        IMUSample imu_sample_new = {};
        memcpy(&imu_sample_new.delta_ang._data[0], delta_ang, sizeof(imu_sample_new.delta_ang._data));
        memcpy(&imu_sample_new.delta_vel._data[0], delta_vel, sizeof(imu_sample_new.delta_vel._data));

        //convert time from us to secs
        imu_sample_new.delta_ang_dt = delta_ang_dt / 1e6f;
        imu_sample_new.delta_vel_dt = delta_vel_dt / 1e6f;
        imu_sample_new.time_us = time_us;
        _imu_ticks++;


        if (collectIMU(imu_sample_new)) {
            _imu_buffer.push(imu_sample_new);
            _imu_ticks = 0;
            _imu_updated = true;

        } else {
            _imu_updated = false;
        }


        _imu_sample_delayed = _imu_buffer.oldest();
    }
    
    void EKFIf::setMagData(uint64_t time_us, float *data)
    {
        if (time_us - _time_last_mag > 70000) {

            MagSample mag_sample_new = {};
            mag_sample_new.time_us = time_us  - _params.mag_delay_ms * 1000;

            mag_sample_new.time_us -= FILTER_UPDATE_PERRIOD_MS * 1000 / 2;
            _time_last_mag = time_us;


            memcpy(&mag_sample_new.mag._data[0], data, sizeof(mag_sample_new.mag._data));

            _mag_buffer.push(mag_sample_new);
        }
    }
    
    void EKFIf::setBaroData(uint64_t time_us, float *data)
    {
        if (!collectBaro(time_us, data) || !_initialised) {
            return;
        }

        if (time_us - _time_last_baro > 70000) {

            BaroSample baro_sample_new;
            baro_sample_new.hgt = *data;
            baro_sample_new.time_us = time_us - _params.baro_delay_ms * 1000;

            baro_sample_new.time_us -= FILTER_UPDATE_PERRIOD_MS * 1000 / 2;
            _time_last_baro = time_us;

            baro_sample_new.time_us = std::max(baro_sample_new.time_us, _imu_sample_delayed.time_us);

            _baro_buffer.push(baro_sample_new);
        }
    }

    bool EKFIf::init(uint64_t time_us)
    {
        _dt_imu_avg = 0.0f;
        _imu_time_last = time_us;

        _imu_sample_delayed.delta_ang.setZero();
        _imu_sample_delayed.delta_vel.setZero();
        _imu_sample_delayed.delta_ang_dt = 0.0f;
        _imu_sample_delayed.delta_vel_dt = 0.0f;
        _imu_sample_delayed.time_us = time_us;

        _imu_ticks = 0;

        _initialised = false;

        _time_last_imu = 0;
        _time_last_gps = 0;
        _time_last_mag = 0;
        _time_last_baro = 0;
        _time_last_range = 0;
        _time_last_airspeed = 0;

        memset(&_fault_status, 0, sizeof(_fault_status));
        return true;
        
    }
    
    bool EKFIf::update()
    {
        return true;
    }
    
    Quaternionf EKFIf::getAttitude() const
    {
        return Quaternionf(_output_new.quat_nominal(0), _output_new.quat_nominal(1), _output_new.quat_nominal(2), _output_new.quat_nominal(3));
    }
    
    Vec3f EKFIf::getPosition() const
    {
        return Vec3f(_output_new.pos(0), _output_new.pos(1), _output_new.pos(2));
    }
    
    Vec3f EKFIf::getVelocity() const
    {
        return Vec3f(_output_new.vel(0), _output_new.vel(1), _output_new.vel(2));
    }
    
    Vec3f EKFIf::getMagI() const
    {
        return Vec3f(_output_new.mag_I(0), _output_new.vel(1), _output_new.mag_I(2));
    }
    
    Vec3f EKFIf::getMagB() const
    {
        return Vec3f(_output_new.mag_B(0), _output_new.mag_B(1), _output_new.mag_B(2));
    }

} //namespace ekf2
} //namespace control
} //namespace uquad