#ifndef UQUAD_CONTROL_EKF2_EKF_H
#define UQUAD_CONTROL_EKF2_EKF_H

#include "EKFIf.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    class EKF
        : public EKFIf
    {
    public:
        EKF();
        ~EKF();
        
        bool update();
        
    protected:
        bool initialiseFilter();
        void initialiseCovariance();
        void predictCovariance();
        void limitCovariance();
        void makeSymmetrical();
       
        void constrainStates();
        void predictStates();
        void calculateOutputStates();
        
        void controlFusionModes();
        void calculateVehicleStatus();
        
        void fuse(float *K, float innovation);
        void fuseVelPosHeight();
        void fuseMag();
        void fuseHeading();
        
        
        void resetVelocity();
        void resetPosition();
        
        bool init(uint64_t time_us);
        bool collectIMU(IMUSample &imu);
        
        FilterControlStatus _control_status = {};
        
    private:
        static const uint8_t _k_num_states = 24;
        static constexpr float _k_earth_rate = 0.000072921f;

        StateSample _state;

        bool _filter_initialised;
        bool _earth_rate_initialised;

        bool _fuse_height;	// baro height data should be fused
        bool _fuse_pos;		// gps position data should be fused
        bool _fuse_hor_vel;		// gps horizontal velocity measurement should be fused
        bool _fuse_vert_vel;	// gps vertical velocity measurement should be fused

        uint64_t _time_last_fake_gps;

        uint64_t _time_last_pos_fuse;   // time the last fusion of horizotal position measurements was performed (usec)
        uint64_t _time_last_vel_fuse;   // time the last fusion of velocity measurements was performed (usec)
        uint64_t _time_last_hgt_fuse;   // time the last fusion of height measurements was performed (usec)
        uint64_t _time_last_of_fuse;    // time the last fusion of optical flow measurements were performed (usec)
        Vector2f _last_known_posNE;     // last known local NE position vector (m)
        float _last_disarmed_posD;      // vertical position recorded at arming (m)

        Vector3f _earth_rate_NED;

        matrix::Dcm<float> _R_prev;

        float P[_k_num_states][_k_num_states];	// state covariance matrix

        float _vel_pos_innov[6];	// innovations: 0-2 vel,  3-5 pos
        float _mag_innov[3];		// earth magnetic field innovations
        float _heading_innov;		// heading measurement innovation

        float _vel_pos_innov_var[6]; // innovation variances: 0-2 vel, 3-5 pos
        float _mag_innov_var[3]; // earth magnetic field innovation variance
        float _heading_innov_var; // heading measurement innovation variance

        // complementary filter states
        Vector3f _delta_angle_corr;
        Vector3f _delta_vel_corr;
        Vector3f _vel_corr;
        IMUSample _imu_down_sampled;
        Quaternion _q_down_sampled;

        // variables used for the GPS quality checks
        float _gpsDriftVelN = 0.0f;     // GPS north position derivative (m/s)
        float _gpsDriftVelE = 0.0f;     // GPS east position derivative (m/s)
        float _gps_drift_velD = 0.0f;     // GPS down position derivative (m/s)
        float _gps_velD_diff_filt = 0.0f;   // GPS filtered Down velocity (m/s)
        float _gps_velN_filt = 0.0f;  // GPS filtered North velocity (m/s)
        float _gps_velE_filt = 0.0f;   // GPS filtered East velocity (m/s)
        uint64_t _last_gps_fail_us = 0;   // last system time in usec that the GPS failed it's checks

        // Variables used to publish the WGS-84 location of the EKF local NED origin
        uint64_t _last_gps_origin_time_us = 0;              // time the origin was last set (uSec)
        float _gps_alt_ref = 0.0f;                          // WGS-84 height (m)


        GPSCheckFailStatus _gps_check_fail_status;
    };
    
    
} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_EKF_H