#include "EKF.h"
#include "../../base/Error.h"
#include "../../math/Utils.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    EKF::EKF()
        : EKFIf()
        , _control_status{}
        , _filter_initialised(false)
        , _earth_rate_initialised(false)
        , _fuse_height(false)
        , _fuse_pos(false)
        , _fuse_hor_vel(false)
        , _fuse_vert_vel(false)
        , _time_last_fake_gps(0)
        , _time_last_pos_fuse(0)
        , _time_last_vel_fuse(0)
        , _time_last_hgt_fuse(0)
        , _time_last_of_fuse(0)
        , _vel_pos_innov{}
        , _mag_innov{}
        , _heading_innov{}
        , _vel_pos_innov_var{}
        , _mag_innov_var{}
        , _heading_innov_var{}
    {
        _earth_rate_NED.setZero();
        _R_prev = matrix::Dcm<float>();
        _delta_angle_corr.setZero();
        _delta_vel_corr.setZero();
        _vel_corr.setZero();
        _last_known_posNE.setZero();
    }
    
    EKF::~EKF()
    {
    }
    
    bool EKF::initialiseFilter()
    {
        _state.ang_error.setZero();
        _state.vel.setZero();
        _state.pos.setZero();
        _state.gyro_bias.setZero();
        _state.gyro_scale(0) = _state.gyro_scale(1) = _state.gyro_scale(2) = 1.0f;
        _state.accel_z_bias = 0.0f;
        _state.mag_I.setZero();
        _state.mag_B.setZero();
        _state.wind_vel.setZero();

        // get initial roll and pitch estimate from accel vector, assuming vehicle is static
        Vector3f accel_init = _imu_down_sampled.delta_vel / _imu_down_sampled.delta_vel_dt;

        float pitch = 0.0f;
        float roll = 0.0f;

        if (accel_init.norm() > 0.001f) {
            accel_init.normalize();

            pitch = asinf(accel_init(0));
            roll = -asinf(accel_init(1) / cosf(pitch));
        }
        
        matrix::Euler<float> euler_init(roll, pitch, 0.0f);
        
        // Get the latest magnetic field measurement.
        // If we don't have a measurement then we cannot initialise the filter
        if (_mag_buffer.empty()) {
            return false;
        }
        
        MagSample mag_init = _mag_buffer.newest();
        

        // rotate magnetic field into earth frame assuming zero yaw and estimate yaw angle assuming zero declination
        // TODO use declination if available
        matrix::Dcm<float> R_to_earth_zeroyaw(euler_init);
        Vector3f mag_ef_zeroyaw = R_to_earth_zeroyaw * mag_init.mag;
        float declination = 0.0f;
        euler_init(2) = declination - atan2f(mag_ef_zeroyaw(1), mag_ef_zeroyaw(0));

        // calculate initial quaternion states
        _state.quat_nominal = Quaternion(euler_init);
        _output_new.quat_nominal = _state.quat_nominal;

        // TODO replace this with a conditional test based on fitered angle error states.
        _control_status.flags.angle_align = true;

        // calculate initial earth magnetic field states
        matrix::Dcm<float> R_to_earth(euler_init);
        _state.mag_I = R_to_earth * mag_init.mag;

        resetVelocity();
        resetPosition();

        // initialize vertical position with newest baro measurement
        if (_baro_buffer.empty()) {
            return false;
        }
        BaroSample baro_init = _baro_buffer.newest();

        _state.pos(2) = -baro_init.hgt;
        _output_new.pos(2) = -baro_init.hgt;

        initialiseCovariance();

        return true;
    }
    
    void EKF::initialiseCovariance()
    {
        for (unsigned i = 0; i < _k_num_states; i++) {
            for (unsigned j = 0; j < _k_num_states; j++) {
                P[i][j] = 0.0f;
            }
        }

        // XXX use initial guess for the diagonal elements for the covariance matrix
        // angle error
        P[0][0] = 0.001f;
        P[1][1] = 0.001f;
        P[2][2] = 0.001f;

        // velocity
        P[3][3] = 0.1f;
        P[4][4] = 0.1f;
        P[5][5] = 0.1f;

        // position
        P[6][6] = 0.1f;
        P[7][7] = 0.1f;
        P[8][8] = 0.1f;

        // gyro bias
        P[9][9] = 0.00001f;
        P[10][10] = 0.00001f;
        P[11][11] = 0.00001f;

        // gyro scale
        P[12][12] = 0.0001f;
        P[13][13] = 0.0001f;
        P[14][14] = 0.0001f;

        // accel z bias
        P[15][15] = 0.0001f;

        // earth magnetic field
        P[16][16] = 0.0001f;
        P[17][17] = 0.0001f;
        P[18][18] = 0.0001f;

        // body magnetic field
        P[19][19] = 0.0001f;
        P[20][20] = 0.0001f;
        P[21][21] = 0.0001f;

        // wind
        P[22][22] = 0.01f;
        P[23][23] = 0.01f;
    }
    
    void EKF::predictCovariance()
    {
        // assign intermediate state variables
        float q0 = _state.quat_nominal(0);
        float q1 = _state.quat_nominal(1);
        float q2 = _state.quat_nominal(2);
        float q3 = _state.quat_nominal(3);

        float dax = _imu_sample_delayed.delta_ang(0);
        float day = _imu_sample_delayed.delta_ang(1);
        float daz = _imu_sample_delayed.delta_ang(2);

        float dvx = _imu_sample_delayed.delta_vel(0);
        float dvy = _imu_sample_delayed.delta_vel(1);
        float dvz = _imu_sample_delayed.delta_vel(2);

        float dax_b = _state.gyro_bias(0);
        float day_b = _state.gyro_bias(1);
        float daz_b = _state.gyro_bias(2);

        float dax_s = _state.gyro_scale(0);
        float day_s = _state.gyro_scale(1);
        float daz_s = _state.gyro_scale(2);

        float dvz_b = _state.accel_z_bias;

        float dt = _imu_sample_delayed.delta_vel_dt;

        // compute process noise
        float process_noise[_k_num_states] = {};

        float d_ang_bias_sig = dt * math::constrain(_params.gyro_bias_p_noise, 0.0f, 1e-4f);
        float d_vel_bias_sig = dt * math::constrain(_params.accel_bias_p_noise, 1e-6f, 1e-2f);
        float d_ang_scale_sig = dt * math::constrain(_params.gyro_scale_p_noise, 1e-6f, 1e-2f);
        float mag_I_sig = dt * math::constrain(_params.mag_p_noise, 1e-4f, 1e-1f);
        float mag_B_sig = dt * math::constrain(_params.mag_p_noise, 1e-4f, 1e-1f);
        float wind_vel_sig = dt * math::constrain(_params.wind_vel_p_noise, 0.01f, 1.0f);

        for (unsigned i = 0; i < 9; i++) {
            process_noise[i] = 0.0f;
        }

        for (unsigned i = 9; i < 12; i++) {
            process_noise[i] = math::square(d_ang_bias_sig);
        }

        for (unsigned i = 12; i < 15; i++) {
            process_noise[i] = math::square(d_ang_scale_sig);
        }

        process_noise[15] = math::square(d_vel_bias_sig);

        for (unsigned i = 16; i < 19; i++) {
            process_noise[i] = math::square(mag_I_sig);
        }

        for (unsigned i = 19; i < 22; i++) {
            process_noise[i] = math::square(mag_B_sig);
        }

        for (unsigned i = 22; i < 24; i++) {
            process_noise[i] = math::square(wind_vel_sig);
        }


        // assign input noise
        // inputs to the system are 3 delta angles and 3 delta velocities
        float daxNoise, dayNoise, dazNoise;
        float dvxNoise, dvyNoise, dvzNoise;
        float gyro_noise = math::constrain(_params.gyro_noise, 1e-4f, 1e-2f);
        daxNoise = dayNoise = dazNoise = dt * gyro_noise;
        float accel_noise = math::constrain(_params.accel_noise, 1e-2f, 1.0f);
        dvxNoise = dvyNoise = dvzNoise = dt * accel_noise;

        // predict covarinace matrix

        // intermediate calculations
        float SF[25] = {};
        SF[0] = daz_b / 2 + dazNoise / 2 - (daz * daz_s) / 2;
        SF[1] = day_b / 2 + dayNoise / 2 - (day * day_s) / 2;
        SF[2] = dax_b / 2 + daxNoise / 2 - (dax * dax_s) / 2;
        SF[3] = q3 / 2 - (q0 * SF[0]) / 2 + (q1 * SF[1]) / 2 - (q2 * SF[2]) / 2;
        SF[4] = q0 / 2 - (q1 * SF[2]) / 2 - (q2 * SF[1]) / 2 + (q3 * SF[0]) / 2;
        SF[5] = q1 / 2 + (q0 * SF[2]) / 2 - (q2 * SF[0]) / 2 - (q3 * SF[1]) / 2;
        SF[6] = q3 / 2 + (q0 * SF[0]) / 2 - (q1 * SF[1]) / 2 - (q2 * SF[2]) / 2;
        SF[7] = q0 / 2 - (q1 * SF[2]) / 2 + (q2 * SF[1]) / 2 - (q3 * SF[0]) / 2;
        SF[8] = q0 / 2 + (q1 * SF[2]) / 2 - (q2 * SF[1]) / 2 - (q3 * SF[0]) / 2;
        SF[9] = q2 / 2 + (q0 * SF[1]) / 2 + (q1 * SF[0]) / 2 + (q3 * SF[2]) / 2;
        SF[10] = q2 / 2 - (q0 * SF[1]) / 2 - (q1 * SF[0]) / 2 + (q3 * SF[2]) / 2;
        SF[11] = q2 / 2 + (q0 * SF[1]) / 2 - (q1 * SF[0]) / 2 - (q3 * SF[2]) / 2;
        SF[12] = q1 / 2 + (q0 * SF[2]) / 2 + (q2 * SF[0]) / 2 + (q3 * SF[1]) / 2;
        SF[13] = q1 / 2 - (q0 * SF[2]) / 2 + (q2 * SF[0]) / 2 - (q3 * SF[1]) / 2;
        SF[14] = q3 / 2 + (q0 * SF[0]) / 2 + (q1 * SF[1]) / 2 + (q2 * SF[2]) / 2;
        SF[15] = - math::square(q0) - math::square(q1) - math::square(q2) - math::square(q3);
        SF[16] = dvz_b - dvz + dvzNoise;
        SF[17] = dvx - dvxNoise;
        SF[18] = dvy - dvyNoise;
        SF[19] = math::square(q2);
        SF[20] = SF[19] - math::square(q0) + math::square(q1) - math::square(q3);
        SF[21] = SF[19] + math::square(q0) - math::square(q1) - math::square(q3);
        SF[22] = 2 * q0 * q1 - 2 * q2 * q3;
        SF[23] = SF[19] - math::square(q0) - math::square(q1) + math::square(q3);
        SF[24] = 2 * q1 * q2;

        float SG[5] = {};
        SG[0] = - math::square(q0) - math::square(q1) - math::square(q2) - math::square(q3);
        SG[1] = math::square(q3);
        SG[2] = math::square(q2);
        SG[3] = math::square(q1);
        SG[4] = math::square(q0);

        float SQ[8] = {};
        SQ[0] = - dvyNoise * (2 * q0 * q1 + 2 * q2 * q3) * (SG[1] - SG[2] + SG[3] - SG[4]) - dvzNoise *
            (2 * q0 * q1 - 2 * q2 * q3) * (SG[1] - SG[2] - SG[3] + SG[4]) - dvxNoise * (2 * q0 * q2 - 2 * q1 * q3) *
            (2 * q0 * q3 + 2 * q1 * q2);
        SQ[1] = dvxNoise * (2 * q0 * q2 - 2 * q1 * q3) * (SG[1] + SG[2] - SG[3] - SG[4]) + dvzNoise *
            (2 * q0 * q2 + 2 * q1 * q3) * (SG[1] - SG[2] - SG[3] + SG[4]) - dvyNoise * (2 * q0 * q1 + 2 * q2 * q3) *
            (2 * q0 * q3 - 2 * q1 * q2);
        SQ[2] = dvyNoise * (2 * q0 * q3 - 2 * q1 * q2) * (SG[1] - SG[2] + SG[3] - SG[4]) - dvxNoise *
            (2 * q0 * q3 + 2 * q1 * q2) * (SG[1] + SG[2] - SG[3] - SG[4]) - dvzNoise * (2 * q0 * q1 - 2 * q2 * q3) *
            (2 * q0 * q2 + 2 * q1 * q3);
        SQ[3] = math::square(SG[0]);
        SQ[4] = 2 * q2 * q3;
        SQ[5] = 2 * q1 * q3;
        SQ[6] = 2 * q1 * q2;
        SQ[7] = SG[4];

        float SPP[23] = {};
        SPP[0] = SF[17] * (2 * q0 * q1 + 2 * q2 * q3) + SF[18] * (2 * q0 * q2 - 2 * q1 * q3);
        SPP[1] = SF[18] * (2 * q0 * q2 + 2 * q1 * q3) + SF[16] * (SF[24] - 2 * q0 * q3);
        SPP[2] = 2 * q3 * SF[8] + 2 * q1 * SF[11] - 2 * q0 * SF[14] - 2 * q2 * SF[13];
        SPP[3] = 2 * q1 * SF[7] + 2 * q2 * SF[6] - 2 * q0 * SF[12] - 2 * q3 * SF[10];
        SPP[4] = 2 * q0 * SF[6] - 2 * q3 * SF[7] - 2 * q1 * SF[10] + 2 * q2 * SF[12];
        SPP[5] = 2 * q0 * SF[8] + 2 * q2 * SF[11] + 2 * q1 * SF[13] + 2 * q3 * SF[14];
        SPP[6] = 2 * q0 * SF[7] + 2 * q3 * SF[6] + 2 * q2 * SF[10] + 2 * q1 * SF[12];
        SPP[7] = 2 * q1 * SF[3] - 2 * q2 * SF[4] - 2 * q3 * SF[5] + 2 * q0 * SF[9];
        SPP[8] = 2 * q0 * SF[5] - 2 * q1 * SF[4] - 2 * q2 * SF[3] + 2 * q3 * SF[9];
        SPP[9] = SF[18] * SF[20] - SF[16] * (2 * q0 * q1 + 2 * q2 * q3);
        SPP[10] = SF[17] * SF[20] + SF[16] * (2 * q0 * q2 - 2 * q1 * q3);
        SPP[11] = SF[17] * SF[21] - SF[18] * (SF[24] + 2 * q0 * q3);
        SPP[12] = SF[17] * SF[22] - SF[16] * (SF[24] + 2 * q0 * q3);
        SPP[13] = 2 * q0 * SF[4] + 2 * q1 * SF[5] + 2 * q3 * SF[3] + 2 * q2 * SF[9];
        SPP[14] = 2 * q2 * SF[8] - 2 * q0 * SF[11] - 2 * q1 * SF[14] + 2 * q3 * SF[13];
        SPP[15] = SF[18] * SF[23] + SF[17] * (SF[24] - 2 * q0 * q3);
        SPP[16] = daz * SF[19] + daz * math::square(q0) + daz * math::square(q1) + daz * math::square(q3);
        SPP[17] = day * SF[19] + day * math::square(q0) + day * math::square(q1) + day * math::square(q3);
        SPP[18] = dax * SF[19] + dax * math::square(q0) + dax * math::square(q1) + dax * math::square(q3);
        SPP[19] = SF[16] * SF[23] - SF[17] * (2 * q0 * q2 + 2 * q1 * q3);
        SPP[20] = SF[16] * SF[21] - SF[18] * SF[22];
        SPP[21] = 2 * q0 * q2 + 2 * q1 * q3;
        SPP[22] = SF[15];

        // covariance update
        float nextP[24][24] = {};
        nextP[0][0] = daxNoise * SQ[3] + SPP[5] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] +
                P[12][0] * SPP[18]) - SPP[4] * (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] *
                        SPP[18]) + SPP[7] * (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]) +
                  SPP[22] * (P[0][9] * SPP[5] - P[1][9] * SPP[4] + P[2][9] * SPP[7] + P[9][9] * SPP[22] + P[12][9] * SPP[18]) +
                  SPP[18] * (P[0][12] * SPP[5] - P[1][12] * SPP[4] + P[2][12] * SPP[7] + P[9][12] * SPP[22] + P[12][12] * SPP[18]);
        nextP[0][1] = SPP[6] * (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] * SPP[18])
                  - SPP[2] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] + P[12][0] * SPP[18]) -
                  SPP[8] * (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]) + SPP[22] *
                  (P[0][10] * SPP[5] - P[1][10] * SPP[4] + P[2][10] * SPP[7] + P[9][10] * SPP[22] + P[12][10] * SPP[18]) + SPP[17] *
                  (P[0][13] * SPP[5] - P[1][13] * SPP[4] + P[2][13] * SPP[7] + P[9][13] * SPP[22] + P[12][13] * SPP[18]);
        nextP[1][1] = dayNoise * SQ[3] - SPP[2] * (P[1][0] * SPP[6] - P[0][0] * SPP[2] - P[2][0] * SPP[8] + P[10][0] * SPP[22] +
                P[13][0] * SPP[17]) + SPP[6] * (P[1][1] * SPP[6] - P[0][1] * SPP[2] - P[2][1] * SPP[8] + P[10][1] * SPP[22] + P[13][1] *
                        SPP[17]) - SPP[8] * (P[1][2] * SPP[6] - P[0][2] * SPP[2] - P[2][2] * SPP[8] + P[10][2] * SPP[22] + P[13][2] * SPP[17]) +
                  SPP[22] * (P[1][10] * SPP[6] - P[0][10] * SPP[2] - P[2][10] * SPP[8] + P[10][10] * SPP[22] + P[13][10] * SPP[17]) +
                  SPP[17] * (P[1][13] * SPP[6] - P[0][13] * SPP[2] - P[2][13] * SPP[8] + P[10][13] * SPP[22] + P[13][13] * SPP[17]);
        nextP[0][2] = SPP[14] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] + P[12][0] *
                     SPP[18]) - SPP[3] * (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] * SPP[18]) +
                  SPP[13] * (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]) +
                  SPP[22] * (P[0][11] * SPP[5] - P[1][11] * SPP[4] + P[2][11] * SPP[7] + P[9][11] * SPP[22] + P[12][11] * SPP[18]) +
                  SPP[16] * (P[0][14] * SPP[5] - P[1][14] * SPP[4] + P[2][14] * SPP[7] + P[9][14] * SPP[22] + P[12][14] * SPP[18]);
        nextP[1][2] = SPP[14] * (P[1][0] * SPP[6] - P[0][0] * SPP[2] - P[2][0] * SPP[8] + P[10][0] * SPP[22] + P[13][0] *
                     SPP[17]) - SPP[3] * (P[1][1] * SPP[6] - P[0][1] * SPP[2] - P[2][1] * SPP[8] + P[10][1] * SPP[22] + P[13][1] * SPP[17]) +
                  SPP[13] * (P[1][2] * SPP[6] - P[0][2] * SPP[2] - P[2][2] * SPP[8] + P[10][2] * SPP[22] + P[13][2] * SPP[17]) +
                  SPP[22] * (P[1][11] * SPP[6] - P[0][11] * SPP[2] - P[2][11] * SPP[8] + P[10][11] * SPP[22] + P[13][11] * SPP[17]) +
                  SPP[16] * (P[1][14] * SPP[6] - P[0][14] * SPP[2] - P[2][14] * SPP[8] + P[10][14] * SPP[22] + P[13][14] * SPP[17]);
        nextP[2][2] = dazNoise * SQ[3] - SPP[3] * (P[0][1] * SPP[14] - P[1][1] * SPP[3] + P[2][1] * SPP[13] + P[11][1] * SPP[22]
                + P[14][1] * SPP[16]) + SPP[14] * (P[0][0] * SPP[14] - P[1][0] * SPP[3] + P[2][0] * SPP[13] + P[11][0] * SPP[22] +
                        P[14][0] * SPP[16]) + SPP[13] * (P[0][2] * SPP[14] - P[1][2] * SPP[3] + P[2][2] * SPP[13] + P[11][2] * SPP[22] +
                                P[14][2] * SPP[16]) + SPP[22] * (P[0][11] * SPP[14] - P[1][11] * SPP[3] + P[2][11] * SPP[13] + P[11][11] * SPP[22] +
                                        P[14][11] * SPP[16]) + SPP[16] * (P[0][14] * SPP[14] - P[1][14] * SPP[3] + P[2][14] * SPP[13] + P[11][14] * SPP[22] +
                                                P[14][14] * SPP[16]);
        nextP[0][3] = P[0][3] * SPP[5] - P[1][3] * SPP[4] + P[2][3] * SPP[7] + P[9][3] * SPP[22] + P[12][3] * SPP[18] +
                  SPP[1] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] + P[12][0] * SPP[18]) + SPP[19] *
                  (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] * SPP[18]) + SPP[15] *
                  (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]) - SPP[21] *
                  (P[0][15] * SPP[5] - P[1][15] * SPP[4] + P[2][15] * SPP[7] + P[9][15] * SPP[22] + P[12][15] * SPP[18]);
        nextP[1][3] = P[1][3] * SPP[6] - P[0][3] * SPP[2] - P[2][3] * SPP[8] + P[10][3] * SPP[22] + P[13][3] * SPP[17] +
                  SPP[1] * (P[1][0] * SPP[6] - P[0][0] * SPP[2] - P[2][0] * SPP[8] + P[10][0] * SPP[22] + P[13][0] * SPP[17]) +
                  SPP[19] * (P[1][1] * SPP[6] - P[0][1] * SPP[2] - P[2][1] * SPP[8] + P[10][1] * SPP[22] + P[13][1] * SPP[17]) +
                  SPP[15] * (P[1][2] * SPP[6] - P[0][2] * SPP[2] - P[2][2] * SPP[8] + P[10][2] * SPP[22] + P[13][2] * SPP[17]) -
                  SPP[21] * (P[1][15] * SPP[6] - P[0][15] * SPP[2] - P[2][15] * SPP[8] + P[10][15] * SPP[22] + P[13][15] * SPP[17]);
        nextP[2][3] = P[0][3] * SPP[14] - P[1][3] * SPP[3] + P[2][3] * SPP[13] + P[11][3] * SPP[22] + P[14][3] * SPP[16] +
                  SPP[1] * (P[0][0] * SPP[14] - P[1][0] * SPP[3] + P[2][0] * SPP[13] + P[11][0] * SPP[22] + P[14][0] * SPP[16]) +
                  SPP[19] * (P[0][1] * SPP[14] - P[1][1] * SPP[3] + P[2][1] * SPP[13] + P[11][1] * SPP[22] + P[14][1] * SPP[16]) +
                  SPP[15] * (P[0][2] * SPP[14] - P[1][2] * SPP[3] + P[2][2] * SPP[13] + P[11][2] * SPP[22] + P[14][2] * SPP[16]) -
                  SPP[21] * (P[0][15] * SPP[14] - P[1][15] * SPP[3] + P[2][15] * SPP[13] + P[11][15] * SPP[22] + P[14][15] * SPP[16]);
        nextP[3][3] = P[3][3] + P[0][3] * SPP[1] + P[1][3] * SPP[19] + P[2][3] * SPP[15] - P[15][3] * SPP[21] + dvyNoise * math::square(
                      SQ[6] - 2 * q0 * q3) + dvzNoise * math::square(SQ[5] + 2 * q0 * q2) + SPP[1] * (P[3][0] + P[0][0] * SPP[1] + P[1][0] * SPP[19] +
                              P[2][0] * SPP[15] - P[15][0] * SPP[21]) + SPP[19] * (P[3][1] + P[0][1] * SPP[1] + P[1][1] * SPP[19] + P[2][1] * SPP[15]
                                      - P[15][1] * SPP[21]) + SPP[15] * (P[3][2] + P[0][2] * SPP[1] + P[1][2] * SPP[19] + P[2][2] * SPP[15] - P[15][2] *
                                              SPP[21]) - SPP[21] * (P[3][15] + P[0][15] * SPP[1] + P[1][15] * SPP[19] + P[2][15] * SPP[15] - P[15][15] * SPP[21]) +
                  dvxNoise * math::square(SG[1] + SG[2] - SG[3] - SQ[7]);
        nextP[0][4] = P[0][4] * SPP[5] - P[1][4] * SPP[4] + P[2][4] * SPP[7] + P[9][4] * SPP[22] + P[12][4] * SPP[18] +
                  SF[22] * (P[0][15] * SPP[5] - P[1][15] * SPP[4] + P[2][15] * SPP[7] + P[9][15] * SPP[22] + P[12][15] * SPP[18]) +
                  SPP[12] * (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] * SPP[18]) +
                  SPP[20] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] + P[12][0] * SPP[18]) +
                  SPP[11] * (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]);
        nextP[1][4] = P[1][4] * SPP[6] - P[0][4] * SPP[2] - P[2][4] * SPP[8] + P[10][4] * SPP[22] + P[13][4] * SPP[17] +
                  SF[22] * (P[1][15] * SPP[6] - P[0][15] * SPP[2] - P[2][15] * SPP[8] + P[10][15] * SPP[22] + P[13][15] * SPP[17]) +
                  SPP[12] * (P[1][1] * SPP[6] - P[0][1] * SPP[2] - P[2][1] * SPP[8] + P[10][1] * SPP[22] + P[13][1] * SPP[17]) +
                  SPP[20] * (P[1][0] * SPP[6] - P[0][0] * SPP[2] - P[2][0] * SPP[8] + P[10][0] * SPP[22] + P[13][0] * SPP[17]) +
                  SPP[11] * (P[1][2] * SPP[6] - P[0][2] * SPP[2] - P[2][2] * SPP[8] + P[10][2] * SPP[22] + P[13][2] * SPP[17]);
        nextP[2][4] = P[0][4] * SPP[14] - P[1][4] * SPP[3] + P[2][4] * SPP[13] + P[11][4] * SPP[22] + P[14][4] * SPP[16] +
                  SF[22] * (P[0][15] * SPP[14] - P[1][15] * SPP[3] + P[2][15] * SPP[13] + P[11][15] * SPP[22] + P[14][15] * SPP[16]) +
                  SPP[12] * (P[0][1] * SPP[14] - P[1][1] * SPP[3] + P[2][1] * SPP[13] + P[11][1] * SPP[22] + P[14][1] * SPP[16]) +
                  SPP[20] * (P[0][0] * SPP[14] - P[1][0] * SPP[3] + P[2][0] * SPP[13] + P[11][0] * SPP[22] + P[14][0] * SPP[16]) +
                  SPP[11] * (P[0][2] * SPP[14] - P[1][2] * SPP[3] + P[2][2] * SPP[13] + P[11][2] * SPP[22] + P[14][2] * SPP[16]);
        nextP[3][4] = P[3][4] + SQ[2] + P[0][4] * SPP[1] + P[1][4] * SPP[19] + P[2][4] * SPP[15] - P[15][4] * SPP[21] +
                  SF[22] * (P[3][15] + P[0][15] * SPP[1] + P[1][15] * SPP[19] + P[2][15] * SPP[15] - P[15][15] * SPP[21]) + SPP[12] *
                  (P[3][1] + P[0][1] * SPP[1] + P[1][1] * SPP[19] + P[2][1] * SPP[15] - P[15][1] * SPP[21]) + SPP[20] *
                  (P[3][0] + P[0][0] * SPP[1] + P[1][0] * SPP[19] + P[2][0] * SPP[15] - P[15][0] * SPP[21]) + SPP[11] *
                  (P[3][2] + P[0][2] * SPP[1] + P[1][2] * SPP[19] + P[2][2] * SPP[15] - P[15][2] * SPP[21]);
        nextP[4][4] = P[4][4] + P[15][4] * SF[22] + P[0][4] * SPP[20] + P[1][4] * SPP[12] + P[2][4] * SPP[11] + dvxNoise * math::square(
                      SQ[6] + 2 * q0 * q3) + dvzNoise * math::square(SQ[4] - 2 * q0 * q1) + SF[22] * (P[4][15] + P[15][15] * SF[22] + P[0][15] * SPP[20]
                              + P[1][15] * SPP[12] + P[2][15] * SPP[11]) + SPP[12] * (P[4][1] + P[15][1] * SF[22] + P[0][1] * SPP[20] + P[1][1] *
                                      SPP[12] + P[2][1] * SPP[11]) + SPP[20] * (P[4][0] + P[15][0] * SF[22] + P[0][0] * SPP[20] + P[1][0] * SPP[12] + P[2][0]
                                              * SPP[11]) + SPP[11] * (P[4][2] + P[15][2] * SF[22] + P[0][2] * SPP[20] + P[1][2] * SPP[12] + P[2][2] * SPP[11]) +
                  dvyNoise * math::square(SG[1] - SG[2] + SG[3] - SQ[7]);
        nextP[0][5] = P[0][5] * SPP[5] - P[1][5] * SPP[4] + P[2][5] * SPP[7] + P[9][5] * SPP[22] + P[12][5] * SPP[18] +
                  SF[20] * (P[0][15] * SPP[5] - P[1][15] * SPP[4] + P[2][15] * SPP[7] + P[9][15] * SPP[22] + P[12][15] * SPP[18]) -
                  SPP[9] * (P[0][0] * SPP[5] - P[1][0] * SPP[4] + P[2][0] * SPP[7] + P[9][0] * SPP[22] + P[12][0] * SPP[18]) + SPP[0] *
                  (P[0][2] * SPP[5] - P[1][2] * SPP[4] + P[2][2] * SPP[7] + P[9][2] * SPP[22] + P[12][2] * SPP[18]) + SPP[10] *
                  (P[0][1] * SPP[5] - P[1][1] * SPP[4] + P[2][1] * SPP[7] + P[9][1] * SPP[22] + P[12][1] * SPP[18]);
        nextP[1][5] = P[1][5] * SPP[6] - P[0][5] * SPP[2] - P[2][5] * SPP[8] + P[10][5] * SPP[22] + P[13][5] * SPP[17] +
                  SF[20] * (P[1][15] * SPP[6] - P[0][15] * SPP[2] - P[2][15] * SPP[8] + P[10][15] * SPP[22] + P[13][15] * SPP[17]) -
                  SPP[9] * (P[1][0] * SPP[6] - P[0][0] * SPP[2] - P[2][0] * SPP[8] + P[10][0] * SPP[22] + P[13][0] * SPP[17]) + SPP[0] *
                  (P[1][2] * SPP[6] - P[0][2] * SPP[2] - P[2][2] * SPP[8] + P[10][2] * SPP[22] + P[13][2] * SPP[17]) + SPP[10] *
                  (P[1][1] * SPP[6] - P[0][1] * SPP[2] - P[2][1] * SPP[8] + P[10][1] * SPP[22] + P[13][1] * SPP[17]);
        nextP[2][5] = P[0][5] * SPP[14] - P[1][5] * SPP[3] + P[2][5] * SPP[13] + P[11][5] * SPP[22] + P[14][5] * SPP[16] +
                  SF[20] * (P[0][15] * SPP[14] - P[1][15] * SPP[3] + P[2][15] * SPP[13] + P[11][15] * SPP[22] + P[14][15] * SPP[16]) -
                  SPP[9] * (P[0][0] * SPP[14] - P[1][0] * SPP[3] + P[2][0] * SPP[13] + P[11][0] * SPP[22] + P[14][0] * SPP[16]) +
                  SPP[0] * (P[0][2] * SPP[14] - P[1][2] * SPP[3] + P[2][2] * SPP[13] + P[11][2] * SPP[22] + P[14][2] * SPP[16]) +
                  SPP[10] * (P[0][1] * SPP[14] - P[1][1] * SPP[3] + P[2][1] * SPP[13] + P[11][1] * SPP[22] + P[14][1] * SPP[16]);
        nextP[3][5] = P[3][5] + SQ[1] + P[0][5] * SPP[1] + P[1][5] * SPP[19] + P[2][5] * SPP[15] - P[15][5] * SPP[21] +
                  SF[20] * (P[3][15] + P[0][15] * SPP[1] + P[1][15] * SPP[19] + P[2][15] * SPP[15] - P[15][15] * SPP[21]) - SPP[9] *
                  (P[3][0] + P[0][0] * SPP[1] + P[1][0] * SPP[19] + P[2][0] * SPP[15] - P[15][0] * SPP[21]) + SPP[0] *
                  (P[3][2] + P[0][2] * SPP[1] + P[1][2] * SPP[19] + P[2][2] * SPP[15] - P[15][2] * SPP[21]) + SPP[10] *
                  (P[3][1] + P[0][1] * SPP[1] + P[1][1] * SPP[19] + P[2][1] * SPP[15] - P[15][1] * SPP[21]);
        nextP[4][5] = P[4][5] + SQ[0] + P[15][5] * SF[22] + P[0][5] * SPP[20] + P[1][5] * SPP[12] + P[2][5] * SPP[11] +
                  SF[20] * (P[4][15] + P[15][15] * SF[22] + P[0][15] * SPP[20] + P[1][15] * SPP[12] + P[2][15] * SPP[11]) - SPP[9] *
                  (P[4][0] + P[15][0] * SF[22] + P[0][0] * SPP[20] + P[1][0] * SPP[12] + P[2][0] * SPP[11]) + SPP[0] *
                  (P[4][2] + P[15][2] * SF[22] + P[0][2] * SPP[20] + P[1][2] * SPP[12] + P[2][2] * SPP[11]) + SPP[10] *
                  (P[4][1] + P[15][1] * SF[22] + P[0][1] * SPP[20] + P[1][1] * SPP[12] + P[2][1] * SPP[11]);
        nextP[5][5] = P[5][5] + P[15][5] * SF[20] - P[0][5] * SPP[9] + P[1][5] * SPP[10] + P[2][5] * SPP[0] + dvxNoise * math::square(
                      SQ[5] - 2 * q0 * q2) + dvyNoise * math::square(SQ[4] + 2 * q0 * q1) + SF[20] * (P[5][15] + P[15][15] * SF[20] - P[0][15] * SPP[9]
                              + P[1][15] * SPP[10] + P[2][15] * SPP[0]) - SPP[9] * (P[5][0] + P[15][0] * SF[20] - P[0][0] * SPP[9] + P[1][0] * SPP[10]
                                      + P[2][0] * SPP[0]) + SPP[0] * (P[5][2] + P[15][2] * SF[20] - P[0][2] * SPP[9] + P[1][2] * SPP[10] + P[2][2] * SPP[0]) +
                  SPP[10] * (P[5][1] + P[15][1] * SF[20] - P[0][1] * SPP[9] + P[1][1] * SPP[10] + P[2][1] * SPP[0]) + dvzNoise * math::square(
                      SG[1] - SG[2] - SG[3] + SQ[7]);
        nextP[0][6] = P[0][6] * SPP[5] - P[1][6] * SPP[4] + P[2][6] * SPP[7] + P[9][6] * SPP[22] + P[12][6] * SPP[18] + dt *
                  (P[0][3] * SPP[5] - P[1][3] * SPP[4] + P[2][3] * SPP[7] + P[9][3] * SPP[22] + P[12][3] * SPP[18]);
        nextP[1][6] = P[1][6] * SPP[6] - P[0][6] * SPP[2] - P[2][6] * SPP[8] + P[10][6] * SPP[22] + P[13][6] * SPP[17] + dt *
                  (P[1][3] * SPP[6] - P[0][3] * SPP[2] - P[2][3] * SPP[8] + P[10][3] * SPP[22] + P[13][3] * SPP[17]);
        nextP[2][6] = P[0][6] * SPP[14] - P[1][6] * SPP[3] + P[2][6] * SPP[13] + P[11][6] * SPP[22] + P[14][6] * SPP[16] +
                  dt * (P[0][3] * SPP[14] - P[1][3] * SPP[3] + P[2][3] * SPP[13] + P[11][3] * SPP[22] + P[14][3] * SPP[16]);
        nextP[3][6] = P[3][6] + P[0][6] * SPP[1] + P[1][6] * SPP[19] + P[2][6] * SPP[15] - P[15][6] * SPP[21] + dt *
                  (P[3][3] + P[0][3] * SPP[1] + P[1][3] * SPP[19] + P[2][3] * SPP[15] - P[15][3] * SPP[21]);
        nextP[4][6] = P[4][6] + P[15][6] * SF[22] + P[0][6] * SPP[20] + P[1][6] * SPP[12] + P[2][6] * SPP[11] + dt *
                  (P[4][3] + P[15][3] * SF[22] + P[0][3] * SPP[20] + P[1][3] * SPP[12] + P[2][3] * SPP[11]);
        nextP[5][6] = P[5][6] + P[15][6] * SF[20] - P[0][6] * SPP[9] + P[1][6] * SPP[10] + P[2][6] * SPP[0] + dt *
                  (P[5][3] + P[15][3] * SF[20] - P[0][3] * SPP[9] + P[1][3] * SPP[10] + P[2][3] * SPP[0]);
        nextP[6][6] = P[6][6] + P[3][6] * dt + dt * (P[6][3] + P[3][3] * dt);
        nextP[0][7] = P[0][7] * SPP[5] - P[1][7] * SPP[4] + P[2][7] * SPP[7] + P[9][7] * SPP[22] + P[12][7] * SPP[18] + dt *
                  (P[0][4] * SPP[5] - P[1][4] * SPP[4] + P[2][4] * SPP[7] + P[9][4] * SPP[22] + P[12][4] * SPP[18]);
        nextP[1][7] = P[1][7] * SPP[6] - P[0][7] * SPP[2] - P[2][7] * SPP[8] + P[10][7] * SPP[22] + P[13][7] * SPP[17] + dt *
                  (P[1][4] * SPP[6] - P[0][4] * SPP[2] - P[2][4] * SPP[8] + P[10][4] * SPP[22] + P[13][4] * SPP[17]);
        nextP[2][7] = P[0][7] * SPP[14] - P[1][7] * SPP[3] + P[2][7] * SPP[13] + P[11][7] * SPP[22] + P[14][7] * SPP[16] +
                  dt * (P[0][4] * SPP[14] - P[1][4] * SPP[3] + P[2][4] * SPP[13] + P[11][4] * SPP[22] + P[14][4] * SPP[16]);
        nextP[3][7] = P[3][7] + P[0][7] * SPP[1] + P[1][7] * SPP[19] + P[2][7] * SPP[15] - P[15][7] * SPP[21] + dt *
                  (P[3][4] + P[0][4] * SPP[1] + P[1][4] * SPP[19] + P[2][4] * SPP[15] - P[15][4] * SPP[21]);
        nextP[4][7] = P[4][7] + P[15][7] * SF[22] + P[0][7] * SPP[20] + P[1][7] * SPP[12] + P[2][7] * SPP[11] + dt *
                  (P[4][4] + P[15][4] * SF[22] + P[0][4] * SPP[20] + P[1][4] * SPP[12] + P[2][4] * SPP[11]);
        nextP[5][7] = P[5][7] + P[15][7] * SF[20] - P[0][7] * SPP[9] + P[1][7] * SPP[10] + P[2][7] * SPP[0] + dt *
                  (P[5][4] + P[15][4] * SF[20] - P[0][4] * SPP[9] + P[1][4] * SPP[10] + P[2][4] * SPP[0]);
        nextP[6][7] = P[6][7] + P[3][7] * dt + dt * (P[6][4] + P[3][4] * dt);
        nextP[7][7] = P[7][7] + P[4][7] * dt + dt * (P[7][4] + P[4][4] * dt);
        nextP[0][8] = P[0][8] * SPP[5] - P[1][8] * SPP[4] + P[2][8] * SPP[7] + P[9][8] * SPP[22] + P[12][8] * SPP[18] + dt *
                  (P[0][5] * SPP[5] - P[1][5] * SPP[4] + P[2][5] * SPP[7] + P[9][5] * SPP[22] + P[12][5] * SPP[18]);
        nextP[1][8] = P[1][8] * SPP[6] - P[0][8] * SPP[2] - P[2][8] * SPP[8] + P[10][8] * SPP[22] + P[13][8] * SPP[17] + dt *
                  (P[1][5] * SPP[6] - P[0][5] * SPP[2] - P[2][5] * SPP[8] + P[10][5] * SPP[22] + P[13][5] * SPP[17]);
        nextP[2][8] = P[0][8] * SPP[14] - P[1][8] * SPP[3] + P[2][8] * SPP[13] + P[11][8] * SPP[22] + P[14][8] * SPP[16] +
                  dt * (P[0][5] * SPP[14] - P[1][5] * SPP[3] + P[2][5] * SPP[13] + P[11][5] * SPP[22] + P[14][5] * SPP[16]);
        nextP[3][8] = P[3][8] + P[0][8] * SPP[1] + P[1][8] * SPP[19] + P[2][8] * SPP[15] - P[15][8] * SPP[21] + dt *
                  (P[3][5] + P[0][5] * SPP[1] + P[1][5] * SPP[19] + P[2][5] * SPP[15] - P[15][5] * SPP[21]);
        nextP[4][8] = P[4][8] + P[15][8] * SF[22] + P[0][8] * SPP[20] + P[1][8] * SPP[12] + P[2][8] * SPP[11] + dt *
                  (P[4][5] + P[15][5] * SF[22] + P[0][5] * SPP[20] + P[1][5] * SPP[12] + P[2][5] * SPP[11]);
        nextP[5][8] = P[5][8] + P[15][8] * SF[20] - P[0][8] * SPP[9] + P[1][8] * SPP[10] + P[2][8] * SPP[0] + dt *
                  (P[5][5] + P[15][5] * SF[20] - P[0][5] * SPP[9] + P[1][5] * SPP[10] + P[2][5] * SPP[0]);
        nextP[6][8] = P[6][8] + P[3][8] * dt + dt * (P[6][5] + P[3][5] * dt);
        nextP[7][8] = P[7][8] + P[4][8] * dt + dt * (P[7][5] + P[4][5] * dt);
        nextP[8][8] = P[8][8] + P[5][8] * dt + dt * (P[8][5] + P[5][5] * dt);
        nextP[0][9] = P[0][9] * SPP[5] - P[1][9] * SPP[4] + P[2][9] * SPP[7] + P[9][9] * SPP[22] + P[12][9] * SPP[18];
        nextP[1][9] = P[1][9] * SPP[6] - P[0][9] * SPP[2] - P[2][9] * SPP[8] + P[10][9] * SPP[22] + P[13][9] * SPP[17];
        nextP[2][9] = P[0][9] * SPP[14] - P[1][9] * SPP[3] + P[2][9] * SPP[13] + P[11][9] * SPP[22] + P[14][9] * SPP[16];
        nextP[3][9] = P[3][9] + P[0][9] * SPP[1] + P[1][9] * SPP[19] + P[2][9] * SPP[15] - P[15][9] * SPP[21];
        nextP[4][9] = P[4][9] + P[15][9] * SF[22] + P[0][9] * SPP[20] + P[1][9] * SPP[12] + P[2][9] * SPP[11];
        nextP[5][9] = P[5][9] + P[15][9] * SF[20] - P[0][9] * SPP[9] + P[1][9] * SPP[10] + P[2][9] * SPP[0];
        nextP[6][9] = P[6][9] + P[3][9] * dt;
        nextP[7][9] = P[7][9] + P[4][9] * dt;
        nextP[8][9] = P[8][9] + P[5][9] * dt;
        nextP[9][9] = P[9][9];
        nextP[0][10] = P[0][10] * SPP[5] - P[1][10] * SPP[4] + P[2][10] * SPP[7] + P[9][10] * SPP[22] + P[12][10] * SPP[18];
        nextP[1][10] = P[1][10] * SPP[6] - P[0][10] * SPP[2] - P[2][10] * SPP[8] + P[10][10] * SPP[22] + P[13][10] * SPP[17];
        nextP[2][10] = P[0][10] * SPP[14] - P[1][10] * SPP[3] + P[2][10] * SPP[13] + P[11][10] * SPP[22] + P[14][10] * SPP[16];
        nextP[3][10] = P[3][10] + P[0][10] * SPP[1] + P[1][10] * SPP[19] + P[2][10] * SPP[15] - P[15][10] * SPP[21];
        nextP[4][10] = P[4][10] + P[15][10] * SF[22] + P[0][10] * SPP[20] + P[1][10] * SPP[12] + P[2][10] * SPP[11];
        nextP[5][10] = P[5][10] + P[15][10] * SF[20] - P[0][10] * SPP[9] + P[1][10] * SPP[10] + P[2][10] * SPP[0];
        nextP[6][10] = P[6][10] + P[3][10] * dt;
        nextP[7][10] = P[7][10] + P[4][10] * dt;
        nextP[8][10] = P[8][10] + P[5][10] * dt;
        nextP[9][10] = P[9][10];
        nextP[10][10] = P[10][10];
        nextP[0][11] = P[0][11] * SPP[5] - P[1][11] * SPP[4] + P[2][11] * SPP[7] + P[9][11] * SPP[22] + P[12][11] * SPP[18];
        nextP[1][11] = P[1][11] * SPP[6] - P[0][11] * SPP[2] - P[2][11] * SPP[8] + P[10][11] * SPP[22] + P[13][11] * SPP[17];
        nextP[2][11] = P[0][11] * SPP[14] - P[1][11] * SPP[3] + P[2][11] * SPP[13] + P[11][11] * SPP[22] + P[14][11] * SPP[16];
        nextP[3][11] = P[3][11] + P[0][11] * SPP[1] + P[1][11] * SPP[19] + P[2][11] * SPP[15] - P[15][11] * SPP[21];
        nextP[4][11] = P[4][11] + P[15][11] * SF[22] + P[0][11] * SPP[20] + P[1][11] * SPP[12] + P[2][11] * SPP[11];
        nextP[5][11] = P[5][11] + P[15][11] * SF[20] - P[0][11] * SPP[9] + P[1][11] * SPP[10] + P[2][11] * SPP[0];
        nextP[6][11] = P[6][11] + P[3][11] * dt;
        nextP[7][11] = P[7][11] + P[4][11] * dt;
        nextP[8][11] = P[8][11] + P[5][11] * dt;
        nextP[9][11] = P[9][11];
        nextP[10][11] = P[10][11];
        nextP[11][11] = P[11][11];
        nextP[0][12] = P[0][12] * SPP[5] - P[1][12] * SPP[4] + P[2][12] * SPP[7] + P[9][12] * SPP[22] + P[12][12] * SPP[18];
        nextP[1][12] = P[1][12] * SPP[6] - P[0][12] * SPP[2] - P[2][12] * SPP[8] + P[10][12] * SPP[22] + P[13][12] * SPP[17];
        nextP[2][12] = P[0][12] * SPP[14] - P[1][12] * SPP[3] + P[2][12] * SPP[13] + P[11][12] * SPP[22] + P[14][12] * SPP[16];
        nextP[3][12] = P[3][12] + P[0][12] * SPP[1] + P[1][12] * SPP[19] + P[2][12] * SPP[15] - P[15][12] * SPP[21];
        nextP[4][12] = P[4][12] + P[15][12] * SF[22] + P[0][12] * SPP[20] + P[1][12] * SPP[12] + P[2][12] * SPP[11];
        nextP[5][12] = P[5][12] + P[15][12] * SF[20] - P[0][12] * SPP[9] + P[1][12] * SPP[10] + P[2][12] * SPP[0];
        nextP[6][12] = P[6][12] + P[3][12] * dt;
        nextP[7][12] = P[7][12] + P[4][12] * dt;
        nextP[8][12] = P[8][12] + P[5][12] * dt;
        nextP[9][12] = P[9][12];
        nextP[10][12] = P[10][12];
        nextP[11][12] = P[11][12];
        nextP[12][12] = P[12][12];
        nextP[0][13] = P[0][13] * SPP[5] - P[1][13] * SPP[4] + P[2][13] * SPP[7] + P[9][13] * SPP[22] + P[12][13] * SPP[18];
        nextP[1][13] = P[1][13] * SPP[6] - P[0][13] * SPP[2] - P[2][13] * SPP[8] + P[10][13] * SPP[22] + P[13][13] * SPP[17];
        nextP[2][13] = P[0][13] * SPP[14] - P[1][13] * SPP[3] + P[2][13] * SPP[13] + P[11][13] * SPP[22] + P[14][13] * SPP[16];
        nextP[3][13] = P[3][13] + P[0][13] * SPP[1] + P[1][13] * SPP[19] + P[2][13] * SPP[15] - P[15][13] * SPP[21];
        nextP[4][13] = P[4][13] + P[15][13] * SF[22] + P[0][13] * SPP[20] + P[1][13] * SPP[12] + P[2][13] * SPP[11];
        nextP[5][13] = P[5][13] + P[15][13] * SF[20] - P[0][13] * SPP[9] + P[1][13] * SPP[10] + P[2][13] * SPP[0];
        nextP[6][13] = P[6][13] + P[3][13] * dt;
        nextP[7][13] = P[7][13] + P[4][13] * dt;
        nextP[8][13] = P[8][13] + P[5][13] * dt;
        nextP[9][13] = P[9][13];
        nextP[10][13] = P[10][13];
        nextP[11][13] = P[11][13];
        nextP[12][13] = P[12][13];
        nextP[13][13] = P[13][13];
        nextP[0][14] = P[0][14] * SPP[5] - P[1][14] * SPP[4] + P[2][14] * SPP[7] + P[9][14] * SPP[22] + P[12][14] * SPP[18];
        nextP[1][14] = P[1][14] * SPP[6] - P[0][14] * SPP[2] - P[2][14] * SPP[8] + P[10][14] * SPP[22] + P[13][14] * SPP[17];
        nextP[2][14] = P[0][14] * SPP[14] - P[1][14] * SPP[3] + P[2][14] * SPP[13] + P[11][14] * SPP[22] + P[14][14] * SPP[16];
        nextP[3][14] = P[3][14] + P[0][14] * SPP[1] + P[1][14] * SPP[19] + P[2][14] * SPP[15] - P[15][14] * SPP[21];
        nextP[4][14] = P[4][14] + P[15][14] * SF[22] + P[0][14] * SPP[20] + P[1][14] * SPP[12] + P[2][14] * SPP[11];
        nextP[5][14] = P[5][14] + P[15][14] * SF[20] - P[0][14] * SPP[9] + P[1][14] * SPP[10] + P[2][14] * SPP[0];
        nextP[6][14] = P[6][14] + P[3][14] * dt;
        nextP[7][14] = P[7][14] + P[4][14] * dt;
        nextP[8][14] = P[8][14] + P[5][14] * dt;
        nextP[9][14] = P[9][14];
        nextP[10][14] = P[10][14];
        nextP[11][14] = P[11][14];
        nextP[12][14] = P[12][14];
        nextP[13][14] = P[13][14];
        nextP[14][14] = P[14][14];
        nextP[0][15] = P[0][15] * SPP[5] - P[1][15] * SPP[4] + P[2][15] * SPP[7] + P[9][15] * SPP[22] + P[12][15] * SPP[18];
        nextP[1][15] = P[1][15] * SPP[6] - P[0][15] * SPP[2] - P[2][15] * SPP[8] + P[10][15] * SPP[22] + P[13][15] * SPP[17];
        nextP[2][15] = P[0][15] * SPP[14] - P[1][15] * SPP[3] + P[2][15] * SPP[13] + P[11][15] * SPP[22] + P[14][15] * SPP[16];
        nextP[3][15] = P[3][15] + P[0][15] * SPP[1] + P[1][15] * SPP[19] + P[2][15] * SPP[15] - P[15][15] * SPP[21];
        nextP[4][15] = P[4][15] + P[15][15] * SF[22] + P[0][15] * SPP[20] + P[1][15] * SPP[12] + P[2][15] * SPP[11];
        nextP[5][15] = P[5][15] + P[15][15] * SF[20] - P[0][15] * SPP[9] + P[1][15] * SPP[10] + P[2][15] * SPP[0];
        nextP[6][15] = P[6][15] + P[3][15] * dt;
        nextP[7][15] = P[7][15] + P[4][15] * dt;
        nextP[8][15] = P[8][15] + P[5][15] * dt;
        nextP[9][15] = P[9][15];
        nextP[10][15] = P[10][15];
        nextP[11][15] = P[11][15];
        nextP[12][15] = P[12][15];
        nextP[13][15] = P[13][15];
        nextP[14][15] = P[14][15];
        nextP[15][15] = P[15][15];
        nextP[0][16] = P[0][16] * SPP[5] - P[1][16] * SPP[4] + P[2][16] * SPP[7] + P[9][16] * SPP[22] + P[12][16] * SPP[18];
        nextP[1][16] = P[1][16] * SPP[6] - P[0][16] * SPP[2] - P[2][16] * SPP[8] + P[10][16] * SPP[22] + P[13][16] * SPP[17];
        nextP[2][16] = P[0][16] * SPP[14] - P[1][16] * SPP[3] + P[2][16] * SPP[13] + P[11][16] * SPP[22] + P[14][16] * SPP[16];
        nextP[3][16] = P[3][16] + P[0][16] * SPP[1] + P[1][16] * SPP[19] + P[2][16] * SPP[15] - P[15][16] * SPP[21];
        nextP[4][16] = P[4][16] + P[15][16] * SF[22] + P[0][16] * SPP[20] + P[1][16] * SPP[12] + P[2][16] * SPP[11];
        nextP[5][16] = P[5][16] + P[15][16] * SF[20] - P[0][16] * SPP[9] + P[1][16] * SPP[10] + P[2][16] * SPP[0];
        nextP[6][16] = P[6][16] + P[3][16] * dt;
        nextP[7][16] = P[7][16] + P[4][16] * dt;
        nextP[8][16] = P[8][16] + P[5][16] * dt;
        nextP[9][16] = P[9][16];
        nextP[10][16] = P[10][16];
        nextP[11][16] = P[11][16];
        nextP[12][16] = P[12][16];
        nextP[13][16] = P[13][16];
        nextP[14][16] = P[14][16];
        nextP[15][16] = P[15][16];
        nextP[16][16] = P[16][16];
        nextP[0][17] = P[0][17] * SPP[5] - P[1][17] * SPP[4] + P[2][17] * SPP[7] + P[9][17] * SPP[22] + P[12][17] * SPP[18];
        nextP[1][17] = P[1][17] * SPP[6] - P[0][17] * SPP[2] - P[2][17] * SPP[8] + P[10][17] * SPP[22] + P[13][17] * SPP[17];
        nextP[2][17] = P[0][17] * SPP[14] - P[1][17] * SPP[3] + P[2][17] * SPP[13] + P[11][17] * SPP[22] + P[14][17] * SPP[16];
        nextP[3][17] = P[3][17] + P[0][17] * SPP[1] + P[1][17] * SPP[19] + P[2][17] * SPP[15] - P[15][17] * SPP[21];
        nextP[4][17] = P[4][17] + P[15][17] * SF[22] + P[0][17] * SPP[20] + P[1][17] * SPP[12] + P[2][17] * SPP[11];
        nextP[5][17] = P[5][17] + P[15][17] * SF[20] - P[0][17] * SPP[9] + P[1][17] * SPP[10] + P[2][17] * SPP[0];
        nextP[6][17] = P[6][17] + P[3][17] * dt;
        nextP[7][17] = P[7][17] + P[4][17] * dt;
        nextP[8][17] = P[8][17] + P[5][17] * dt;
        nextP[9][17] = P[9][17];
        nextP[10][17] = P[10][17];
        nextP[11][17] = P[11][17];
        nextP[12][17] = P[12][17];
        nextP[13][17] = P[13][17];
        nextP[14][17] = P[14][17];
        nextP[15][17] = P[15][17];
        nextP[16][17] = P[16][17];
        nextP[17][17] = P[17][17];
        nextP[0][18] = P[0][18] * SPP[5] - P[1][18] * SPP[4] + P[2][18] * SPP[7] + P[9][18] * SPP[22] + P[12][18] * SPP[18];
        nextP[1][18] = P[1][18] * SPP[6] - P[0][18] * SPP[2] - P[2][18] * SPP[8] + P[10][18] * SPP[22] + P[13][18] * SPP[17];
        nextP[2][18] = P[0][18] * SPP[14] - P[1][18] * SPP[3] + P[2][18] * SPP[13] + P[11][18] * SPP[22] + P[14][18] * SPP[16];
        nextP[3][18] = P[3][18] + P[0][18] * SPP[1] + P[1][18] * SPP[19] + P[2][18] * SPP[15] - P[15][18] * SPP[21];
        nextP[4][18] = P[4][18] + P[15][18] * SF[22] + P[0][18] * SPP[20] + P[1][18] * SPP[12] + P[2][18] * SPP[11];
        nextP[5][18] = P[5][18] + P[15][18] * SF[20] - P[0][18] * SPP[9] + P[1][18] * SPP[10] + P[2][18] * SPP[0];
        nextP[6][18] = P[6][18] + P[3][18] * dt;
        nextP[7][18] = P[7][18] + P[4][18] * dt;
        nextP[8][18] = P[8][18] + P[5][18] * dt;
        nextP[9][18] = P[9][18];
        nextP[10][18] = P[10][18];
        nextP[11][18] = P[11][18];
        nextP[12][18] = P[12][18];
        nextP[13][18] = P[13][18];
        nextP[14][18] = P[14][18];
        nextP[15][18] = P[15][18];
        nextP[16][18] = P[16][18];
        nextP[17][18] = P[17][18];
        nextP[18][18] = P[18][18];
        nextP[0][19] = P[0][19] * SPP[5] - P[1][19] * SPP[4] + P[2][19] * SPP[7] + P[9][19] * SPP[22] + P[12][19] * SPP[18];
        nextP[1][19] = P[1][19] * SPP[6] - P[0][19] * SPP[2] - P[2][19] * SPP[8] + P[10][19] * SPP[22] + P[13][19] * SPP[17];
        nextP[2][19] = P[0][19] * SPP[14] - P[1][19] * SPP[3] + P[2][19] * SPP[13] + P[11][19] * SPP[22] + P[14][19] * SPP[16];
        nextP[3][19] = P[3][19] + P[0][19] * SPP[1] + P[1][19] * SPP[19] + P[2][19] * SPP[15] - P[15][19] * SPP[21];
        nextP[4][19] = P[4][19] + P[15][19] * SF[22] + P[0][19] * SPP[20] + P[1][19] * SPP[12] + P[2][19] * SPP[11];
        nextP[5][19] = P[5][19] + P[15][19] * SF[20] - P[0][19] * SPP[9] + P[1][19] * SPP[10] + P[2][19] * SPP[0];
        nextP[6][19] = P[6][19] + P[3][19] * dt;
        nextP[7][19] = P[7][19] + P[4][19] * dt;
        nextP[8][19] = P[8][19] + P[5][19] * dt;
        nextP[9][19] = P[9][19];
        nextP[10][19] = P[10][19];
        nextP[11][19] = P[11][19];
        nextP[12][19] = P[12][19];
        nextP[13][19] = P[13][19];
        nextP[14][19] = P[14][19];
        nextP[15][19] = P[15][19];
        nextP[16][19] = P[16][19];
        nextP[17][19] = P[17][19];
        nextP[18][19] = P[18][19];
        nextP[19][19] = P[19][19];
        nextP[0][20] = P[0][20] * SPP[5] - P[1][20] * SPP[4] + P[2][20] * SPP[7] + P[9][20] * SPP[22] + P[12][20] * SPP[18];
        nextP[1][20] = P[1][20] * SPP[6] - P[0][20] * SPP[2] - P[2][20] * SPP[8] + P[10][20] * SPP[22] + P[13][20] * SPP[17];
        nextP[2][20] = P[0][20] * SPP[14] - P[1][20] * SPP[3] + P[2][20] * SPP[13] + P[11][20] * SPP[22] + P[14][20] * SPP[16];
        nextP[3][20] = P[3][20] + P[0][20] * SPP[1] + P[1][20] * SPP[19] + P[2][20] * SPP[15] - P[15][20] * SPP[21];
        nextP[4][20] = P[4][20] + P[15][20] * SF[22] + P[0][20] * SPP[20] + P[1][20] * SPP[12] + P[2][20] * SPP[11];
        nextP[5][20] = P[5][20] + P[15][20] * SF[20] - P[0][20] * SPP[9] + P[1][20] * SPP[10] + P[2][20] * SPP[0];
        nextP[6][20] = P[6][20] + P[3][20] * dt;
        nextP[7][20] = P[7][20] + P[4][20] * dt;
        nextP[8][20] = P[8][20] + P[5][20] * dt;
        nextP[9][20] = P[9][20];
        nextP[10][20] = P[10][20];
        nextP[11][20] = P[11][20];
        nextP[12][20] = P[12][20];
        nextP[13][20] = P[13][20];
        nextP[14][20] = P[14][20];
        nextP[15][20] = P[15][20];
        nextP[16][20] = P[16][20];
        nextP[17][20] = P[17][20];
        nextP[18][20] = P[18][20];
        nextP[19][20] = P[19][20];
        nextP[20][20] = P[20][20];
        nextP[0][21] = P[0][21] * SPP[5] - P[1][21] * SPP[4] + P[2][21] * SPP[7] + P[9][21] * SPP[22] + P[12][21] * SPP[18];
        nextP[1][21] = P[1][21] * SPP[6] - P[0][21] * SPP[2] - P[2][21] * SPP[8] + P[10][21] * SPP[22] + P[13][21] * SPP[17];
        nextP[2][21] = P[0][21] * SPP[14] - P[1][21] * SPP[3] + P[2][21] * SPP[13] + P[11][21] * SPP[22] + P[14][21] * SPP[16];
        nextP[3][21] = P[3][21] + P[0][21] * SPP[1] + P[1][21] * SPP[19] + P[2][21] * SPP[15] - P[15][21] * SPP[21];
        nextP[4][21] = P[4][21] + P[15][21] * SF[22] + P[0][21] * SPP[20] + P[1][21] * SPP[12] + P[2][21] * SPP[11];
        nextP[5][21] = P[5][21] + P[15][21] * SF[20] - P[0][21] * SPP[9] + P[1][21] * SPP[10] + P[2][21] * SPP[0];
        nextP[6][21] = P[6][21] + P[3][21] * dt;
        nextP[7][21] = P[7][21] + P[4][21] * dt;
        nextP[8][21] = P[8][21] + P[5][21] * dt;
        nextP[9][21] = P[9][21];
        nextP[10][21] = P[10][21];
        nextP[11][21] = P[11][21];
        nextP[12][21] = P[12][21];
        nextP[13][21] = P[13][21];
        nextP[14][21] = P[14][21];
        nextP[15][21] = P[15][21];
        nextP[16][21] = P[16][21];
        nextP[17][21] = P[17][21];
        nextP[18][21] = P[18][21];
        nextP[19][21] = P[19][21];
        nextP[20][21] = P[20][21];
        nextP[21][21] = P[21][21];
        nextP[0][22] = P[0][22] * SPP[5] - P[1][22] * SPP[4] + P[2][22] * SPP[7] + P[9][22] * SPP[22] + P[12][22] * SPP[18];
        nextP[1][22] = P[1][22] * SPP[6] - P[0][22] * SPP[2] - P[2][22] * SPP[8] + P[10][22] * SPP[22] + P[13][22] * SPP[17];
        nextP[2][22] = P[0][22] * SPP[14] - P[1][22] * SPP[3] + P[2][22] * SPP[13] + P[11][22] * SPP[22] + P[14][22] * SPP[16];
        nextP[3][22] = P[3][22] + P[0][22] * SPP[1] + P[1][22] * SPP[19] + P[2][22] * SPP[15] - P[15][22] * SPP[21];
        nextP[4][22] = P[4][22] + P[15][22] * SF[22] + P[0][22] * SPP[20] + P[1][22] * SPP[12] + P[2][22] * SPP[11];
        nextP[5][22] = P[5][22] + P[15][22] * SF[20] - P[0][22] * SPP[9] + P[1][22] * SPP[10] + P[2][22] * SPP[0];
        nextP[6][22] = P[6][22] + P[3][22] * dt;
        nextP[7][22] = P[7][22] + P[4][22] * dt;
        nextP[8][22] = P[8][22] + P[5][22] * dt;
        nextP[9][22] = P[9][22];
        nextP[10][22] = P[10][22];
        nextP[11][22] = P[11][22];
        nextP[12][22] = P[12][22];
        nextP[13][22] = P[13][22];
        nextP[14][22] = P[14][22];
        nextP[15][22] = P[15][22];
        nextP[16][22] = P[16][22];
        nextP[17][22] = P[17][22];
        nextP[18][22] = P[18][22];
        nextP[19][22] = P[19][22];
        nextP[20][22] = P[20][22];
        nextP[21][22] = P[21][22];
        nextP[22][22] = P[22][22];
        nextP[0][23] = P[0][23] * SPP[5] - P[1][23] * SPP[4] + P[2][23] * SPP[7] + P[9][23] * SPP[22] + P[12][23] * SPP[18];
        nextP[1][23] = P[1][23] * SPP[6] - P[0][23] * SPP[2] - P[2][23] * SPP[8] + P[10][23] * SPP[22] + P[13][23] * SPP[17];
        nextP[2][23] = P[0][23] * SPP[14] - P[1][23] * SPP[3] + P[2][23] * SPP[13] + P[11][23] * SPP[22] + P[14][23] * SPP[16];
        nextP[3][23] = P[3][23] + P[0][23] * SPP[1] + P[1][23] * SPP[19] + P[2][23] * SPP[15] - P[15][23] * SPP[21];
        nextP[4][23] = P[4][23] + P[15][23] * SF[22] + P[0][23] * SPP[20] + P[1][23] * SPP[12] + P[2][23] * SPP[11];
        nextP[5][23] = P[5][23] + P[15][23] * SF[20] - P[0][23] * SPP[9] + P[1][23] * SPP[10] + P[2][23] * SPP[0];
        nextP[6][23] = P[6][23] + P[3][23] * dt;
        nextP[7][23] = P[7][23] + P[4][23] * dt;
        nextP[8][23] = P[8][23] + P[5][23] * dt;
        nextP[9][23] = P[9][23];
        nextP[10][23] = P[10][23];
        nextP[11][23] = P[11][23];
        nextP[12][23] = P[12][23];
        nextP[13][23] = P[13][23];
        nextP[14][23] = P[14][23];
        nextP[15][23] = P[15][23];
        nextP[16][23] = P[16][23];
        nextP[17][23] = P[17][23];
        nextP[18][23] = P[18][23];
        nextP[19][23] = P[19][23];
        nextP[20][23] = P[20][23];
        nextP[21][23] = P[21][23];
        nextP[22][23] = P[22][23];
        nextP[23][23] = P[23][23];

        // add process noise
        for (unsigned i = 0; i < _k_num_states; i++) {
            nextP[i][i] += process_noise[i];
        }

        // stop position covariance growth if our total position variance reaches 100m
        // this can happen if we loose gps for some time
        if ((P[6][6] + P[7][7]) > 1e4f) {
            for (uint8_t i = 6; i < 8; i++) {
                for (uint8_t j = 0; j < _k_num_states; j++) {
                    nextP[i][j] = P[i][j];
                    nextP[j][i] = P[j][i];
                }
            }
        }

        // covariance matrix is symmetrical, so copy upper half to lower half
        for (unsigned row = 1; row < _k_num_states; row++) {
            for (unsigned column = 0 ; column < row; column++) {
                nextP[row][column] = nextP[column][row];
            }
        }

        for (unsigned i = 0; i < _k_num_states; i++) {
            P[i][i] = nextP[i][i];
        }

        for (unsigned row = 1; row < _k_num_states; row++) {
            for (unsigned column = 0; column < row; column++) {
                P[row][column] = 0.5f * (nextP[row][column] + nextP[column][row]);
                P[column][row] = P[row][column];
            }
        }

        limitCovariance();
    }

    void EKF::limitCovariance()
    {
        // Covariance diagonal limits. Use same values for states which
        // belong to the same group (e.g. vel_x, vel_y, vel_z)
        float P_lim[9] = {};
        P_lim[0] = 1.0f;		// angle error max var
        P_lim[1] = 1000.0f;		// velocity max var
        P_lim[2] = 1000000.0f;		// positiion max var
        P_lim[3] = 0.001f;		// gyro bias max var
        P_lim[4] = 0.01f;		// gyro scale max var
        P_lim[5] = 0.1f;		// delta velocity z bias max var
        P_lim[6] = 0.01f;		// earth mag field max var
        P_lim[7] = 0.01f;		// body mag field max var
        P_lim[8] = 1000.0f;		// wind max var

        for (int i = 0; i < 3; i++) {

            math::constrain(P[i][i], 0.0f, P_lim[0]);
        }

        for (int i = 3; i < 6; i++) {

            math::constrain(P[i][i], 0.0f, P_lim[1]);
        }

        for (int i = 6; i < 9; i++) {


            math::constrain(P[i][i], 0.0f, P_lim[2]);
        }

        for (int i = 9; i < 12; i++) {


            math::constrain(P[i][i], 0.0f, P_lim[3]);
        }

        for (int i = 12; i < 15; i++) {


            math::constrain(P[i][i], 0.0f, P_lim[4]);
        }


        math::constrain(P[15][15], 0.0f, P_lim[5]);

        for (int i = 16; i < 19; i++) {

            math::constrain(P[i][i], 0.0f, P_lim[6]);
        }

        for (int i = 19; i < 22; i++) {


            math::constrain(P[i][i], 0.0f, P_lim[7]);
        }

        for (int i = 22; i < 24; i++) {

            math::constrain(P[i][i], 0.0f, P_lim[8]);
        }
    }
    
    void EKF::makeSymmetrical()
    {
        for (unsigned row = 0; row < _k_num_states; row++) {
            for (unsigned column = 0; column < row; column++) {
                float tmp = (P[row][column] + P[column][row]) / 2;
                P[row][column] = tmp;
                P[column][row] = tmp;
            }
        }
    }
    
    void EKF::constrainStates()
    {
        for (int i = 0; i < 3; i++) {
            _state.ang_error(i) = math::constrain(_state.ang_error(i), -1.0f, 1.0f);
        }

        for (int i = 0; i < 3; i++) {
            _state.vel(i) = math::constrain(_state.vel(i), -1000.0f, 1000.0f);
        }

        for (int i = 0; i < 3; i++) {
            _state.pos(i) = math::constrain(_state.pos(i), -1.e6f, 1.e6f);
        }

        for (int i = 0; i < 3; i++) {
            _state.gyro_bias(i) = math::constrain(_state.gyro_bias(i), -0.349066f * _dt_imu_avg, 0.349066f * _dt_imu_avg);
        }

        for (int i = 0; i < 3; i++) {
            _state.gyro_scale(i) = math::constrain(_state.gyro_scale(i), 0.95f, 1.05f);
        }

        _state.accel_z_bias = math::constrain(_state.accel_z_bias, -1.0f * _dt_imu_avg, 1.0f * _dt_imu_avg);

        for (int i = 0; i < 3; i++) {
            _state.mag_I(i) = math::constrain(_state.mag_I(i), -1.0f, 1.0f);
        }

        for (int i = 0; i < 3; i++) {
            _state.mag_B(i) = math::constrain(_state.mag_B(i), -0.5f, 0.5f);
        }

        for (int i = 0; i < 2; i++) {
            _state.wind_vel(i) = math::constrain(_state.wind_vel(i), -100.0f, 100.0f);
        }
    }
    
    void EKF::predictStates()
    {
        /*if (!_earth_rate_initialised) {
            if (_NED_origin_initialised) {
                calcEarthRateNED(_earth_rate_NED, _pos_ref.lat_rad);
                _earth_rate_initialised = true;
            }
        }*/

        // attitude error state prediciton
        matrix::Dcm<float> R_to_earth(_state.quat_nominal);	// transformation matrix from body to world frame
        Vector3f corrected_delta_ang = _imu_sample_delayed.delta_ang - _R_prev * _earth_rate_NED *
                           _imu_sample_delayed.delta_ang_dt;
        Quaternion dq;	// delta quaternion since last update
        dq.from_axis_angle(corrected_delta_ang);
        _state.quat_nominal = dq * _state.quat_nominal;
        _state.quat_nominal.normalize();

        _R_prev = R_to_earth.transpose();

        Vector3f vel_last = _state.vel;

        // predict velocity states
        _state.vel += R_to_earth * _imu_sample_delayed.delta_vel;
        _state.vel(2) += 9.81f * _imu_sample_delayed.delta_vel_dt;

        // predict position states via trapezoidal integration of velocity
        _state.pos += (vel_last + _state.vel) * _imu_sample_delayed.delta_vel_dt * 0.5f;

        constrainStates();
    }
    
    void EKF::calculateOutputStates()
    {
        IMUSample imu_new = _imu_sample_new;
        Vector3f delta_angle;

        // Note: We do no not need to consider any bias or scale correction here
        // since the base class has already corrected the imu sample
        delta_angle(0) = imu_new.delta_ang(0);
        delta_angle(1) = imu_new.delta_ang(1);
        delta_angle(2) = imu_new.delta_ang(2);

        Vector3f delta_vel = imu_new.delta_vel;

        delta_angle += _delta_angle_corr;
        Quaternion dq;
        dq.from_axis_angle(delta_angle);

        _output_new.time_us = imu_new.time_us;
        _output_new.quat_nominal = dq * _output_new.quat_nominal;
        _output_new.quat_nominal.normalize();

        matrix::Dcm<float> R_to_earth(_output_new.quat_nominal);

        Vector3f delta_vel_NED = R_to_earth * delta_vel + _delta_vel_corr;
        delta_vel_NED(2) += 9.81f * imu_new.delta_vel_dt;

        Vector3f vel_last = _output_new.vel;

        _output_new.vel += delta_vel_NED;

        _output_new.pos += (_output_new.vel + vel_last) * (imu_new.delta_vel_dt * 0.5f) + _vel_corr * imu_new.delta_vel_dt;

        if (_imu_updated) {
            _output_buffer.push(_output_new);
            _imu_updated = false;
        }

        _output_sample_delayed = _output_buffer.oldest();

        Quaternion quat_inv = _state.quat_nominal.inversed();
        Quaternion q_error =  _output_sample_delayed.quat_nominal * quat_inv;
        q_error.normalize();
        Vector3f delta_ang_error;

        float scalar;

        if (q_error(0) >= 0.0f) {
            scalar = -2.0f;

        } else {
            scalar = 2.0f;
        }

        delta_ang_error(0) = scalar * q_error(1);
        delta_ang_error(1) = scalar * q_error(2);
        delta_ang_error(2) = scalar * q_error(3);

        _delta_angle_corr = delta_ang_error * imu_new.delta_ang_dt;

        _delta_vel_corr = (_state.vel - _output_sample_delayed.vel) * imu_new.delta_vel_dt;

        _vel_corr = (_state.pos - _output_sample_delayed.pos);
    }
    
    void EKF::controlFusionModes()
    {
        // Determine the vehicle status
        calculateVehicleStatus();

        // optical flow fusion mode selection logic
        _control_status.flags.opt_flow = false;

        // GPS fusion mode selection logic
        // To start use GPS we need angular alignment completed, the local NED origin set and fresh GPS data
        if (!_control_status.flags.gps) {
            if (_control_status.flags.angle_align && (_time_last_imu - _time_last_gps) < 5e5 && _NED_origin_initialised
                && (_time_last_imu - _last_gps_fail_us > 5e6)) {
                _control_status.flags.gps = true;
                resetPosition();
                resetVelocity();
            }
        }

        // decide when to start using optical flow data
        if (!_control_status.flags.opt_flow) {
            // TODO optical flow start logic
        }

        // handle the case when we are relying on GPS fusion and lose it
        if (_control_status.flags.gps && !_control_status.flags.opt_flow) {
            // We are relying on GPS aiding to constrain attitude drift so after 10 seconds without aiding we need to do something
            if ((_time_last_imu - _time_last_pos_fuse > 10e6) && (_time_last_imu - _time_last_vel_fuse > 10e6)) {
                if (_time_last_imu - _time_last_gps > 5e5) {
                    // if we don't have gps then we need to switch to the non-aiding mode, zero the veloity states
                    // and set the synthetic GPS position to the current estimate
                    _control_status.flags.gps = false;
                    _last_known_posNE(0) = _state.pos(0);
                    _last_known_posNE(1) = _state.pos(1);
                    _state.vel.setZero();

                } else {
                    // Reset states to the last GPS measurement
                    resetPosition();
                    resetVelocity();
                }
            }
        }

        // handle the case when we are relying on optical flow fusion and lose it
        if (_control_status.flags.opt_flow && !_control_status.flags.gps) {
            // TODO
        }

        // Determine if we should use simple magnetic heading fusion which works better when there are large external disturbances
        // or the more accurate 3-axis fusion
        if (!_control_status.flags.armed) {
            // always use simple mag fusion for initial startup
            _control_status.flags.mag_hdg = true;
            _control_status.flags.mag_3D = false;

        } else {
            if (_control_status.flags.in_air) {
                // always use 3-axis mag fusion when airborne
                _control_status.flags.mag_hdg = false;
                _control_status.flags.mag_3D = true;

            } else {
                // always use simple heading fusion when on the ground
                _control_status.flags.mag_hdg = true;
                _control_status.flags.mag_3D = false;
            }
        }
    }

    void EKF::calculateVehicleStatus()
    {
        // determine if the vehicle is armed
        _control_status.flags.armed = _vehicle_armed;

        // record vertical position whilst disarmed to use as a height change reference
        if (!_control_status.flags.armed) {
            _last_disarmed_posD = _state.pos(2);
        }

        // Transition to in-air occurs when armed and when altitude has increased sufficiently from the altitude at arming
        if (!_control_status.flags.in_air && _control_status.flags.armed && (_state.pos(2) - _last_disarmed_posD) < -1.0f) {
            _control_status.flags.in_air = true;
        }

        // Transition to on-ground occurs when disarmed.
        if (_control_status.flags.in_air && !_control_status.flags.armed) {
            _control_status.flags.in_air = false;
        }
    }
    
    void EKF::fuse(float *K, float innovation)
    {
        for (unsigned i = 0; i < 3; i++) {
            _state.ang_error(i) = _state.ang_error(i) - K[i] * innovation;
        }

        for (unsigned i = 0; i < 3; i++) {
            _state.vel(i) = _state.vel(i) - K[i + 3] * innovation;
        }

        for (unsigned i = 0; i < 3; i++) {
            _state.pos(i) = _state.pos(i) - K[i + 6] * innovation;
        }

        for (unsigned i = 0; i < 3; i++) {
            _state.gyro_bias(i) = _state.gyro_bias(i) - K[i + 9] * innovation;
        }

        for (unsigned i = 0; i < 3; i++) {
            _state.gyro_scale(i) = _state.gyro_scale(i) - K[i + 12] * innovation;
        }

        _state.accel_z_bias -= K[15] * innovation;

        for (unsigned i = 0; i < 3; i++) {
            _state.mag_I(i) = _state.mag_I(i) - K[i + 16] * innovation;
        }

        for (unsigned i = 0; i < 3; i++) {
            _state.mag_B(i) = _state.mag_B(i) - K[i + 19] * innovation;
        }

        for (unsigned i = 0; i < 2; i++) {
            _state.wind_vel(i) = _state.wind_vel(i) - K[i + 22] * innovation;
        }
    }
    
    void EKF::fuseVelPosHeight()
    {
        bool fuse_map[6] = {};
        bool innov_check_pass_map[6] = {};
        float R[6] = {};
        float gate_size[6] = {};
        float Kfusion[24] = {};

        // calculate innovations and gate sizes
        if (_fuse_hor_vel) {
            fuse_map[0] = fuse_map[1] = true;
            _vel_pos_innov[0] = _state.vel(0) - _gps_sample_delayed.vel(0);
            _vel_pos_innov[1] = _state.vel(1) - _gps_sample_delayed.vel(1);
            R[0] = _params.gps_vel_noise;
            R[1] = _params.gps_vel_noise;
            gate_size[0] = fmaxf(_params.vel_innov_gate, 1.0f);
            gate_size[1] = gate_size[0];
        }

        if (_fuse_vert_vel) {
            fuse_map[2] = true;
            _vel_pos_innov[2] = _state.vel(2) - _gps_sample_delayed.vel(2);
            R[2] = _params.gps_vel_noise;
            gate_size[2] = fmaxf(_params.vel_innov_gate, 1.0f);
        }

        if (_fuse_pos) {
            fuse_map[3] = fuse_map[4] = true;
            _vel_pos_innov[3] = _state.pos(0) - _gps_sample_delayed.pos(0);
            _vel_pos_innov[4] = _state.pos(1) - _gps_sample_delayed.pos(1);
            R[3] = _params.gps_pos_noise;
            R[4] = _params.gps_pos_noise;
            gate_size[3] = fmaxf(_params.posNE_innov_gate, 1.0f);
            gate_size[4] = gate_size[3];
        }

        if (_fuse_height) {
            fuse_map[5] = true;
            _vel_pos_innov[5] = _state.pos(2) - (-_baro_sample_delayed.hgt);		// baro measurement has inversed z axis
            R[5] = _params.baro_noise;
            gate_size[5] = fmaxf(_params.baro_innov_gate, 1.0f);
        }

        // calculate innovation test ratios
        for (unsigned obs_index = 0; obs_index < 6; obs_index++) {
            if (fuse_map[obs_index]) {
                // compute the innovation variance SK = HPH + R
                unsigned state_index = obs_index + 3;	// we start with vx and this is the 4. state
                _vel_pos_innov_var[obs_index] = P[state_index][state_index] + R[obs_index];
                // Compute the ratio of innovation to gate size
                _vel_pos_test_ratio[obs_index] = math::square(_vel_pos_innov[obs_index]) / (math::square(gate_size[obs_index]) * _vel_pos_innov[obs_index]);
            }
        }

        // check position, velocity and height innovations
        // treat 3D velocity, 2D position and height as separate sensors
        // always pass position checks if using synthetic position measurements
        bool vel_check_pass = (_vel_pos_test_ratio[0] <= 1.0f) && (_vel_pos_test_ratio[1] <= 1.0f) && (_vel_pos_test_ratio[2] <= 1.0f);
        innov_check_pass_map[2] = innov_check_pass_map[1] = innov_check_pass_map[0] = vel_check_pass;
        bool using_synthetic_measurements = !_control_status.flags.gps && !_control_status.flags.opt_flow;
        bool pos_check_pass = ((_vel_pos_test_ratio[3] <= 1.0f) && (_vel_pos_test_ratio[4] <= 1.0f)) || using_synthetic_measurements;
        innov_check_pass_map[4] = innov_check_pass_map[3] = pos_check_pass;
        innov_check_pass_map[5] = (_vel_pos_test_ratio[5] <= 1.0f);

        // record the successful velocity fusion time
        if (vel_check_pass && _fuse_hor_vel) {
            _time_last_vel_fuse = _time_last_imu;
        }

        // record the successful position fusion time
        if (pos_check_pass && _fuse_pos) {
            _time_last_pos_fuse = _time_last_imu;
        }

        // record the successful height fusion time
        if (innov_check_pass_map[5] && _fuse_height) {
            _time_last_hgt_fuse = _time_last_imu;
        }

        for (unsigned obs_index = 0; obs_index < 6; obs_index++) {
            // skip fusion if not requested or checks have failed
            if (!fuse_map[obs_index] || !innov_check_pass_map[obs_index]) {
                continue;
            }

            unsigned state_index = obs_index + 3;	// we start with vx and this is the 4. state

            // calculate kalman gain K = PHS, where S = 1/innovation variance
            for (int row = 0; row < 24; row++) {
                Kfusion[row] = P[row][state_index] / _vel_pos_innov_var[obs_index];
            }

            // by definition the angle error state is zero at the fusion time
            _state.ang_error.setZero();

            // fuse the observation
            fuse(Kfusion, _vel_pos_innov[obs_index]);

            // correct the nominal quaternion
            Quaternion dq;
            dq.from_axis_angle(_state.ang_error);
            _state.quat_nominal = dq * _state.quat_nominal;
            _state.quat_nominal.normalize();

            // update covarinace matrix via Pnew = (I - KH)P
            float KHP[_k_num_states][_k_num_states] = {};

            for (unsigned row = 0; row < _k_num_states; row++) {
                for (unsigned column = 0; column < _k_num_states; column++) {
                    KHP[row][column] = Kfusion[row] * P[state_index][column];
                }
            }

            for (unsigned row = 0; row < _k_num_states; row++) {
                for (unsigned column = 0; column < _k_num_states; column++) {
                    P[row][column] = P[row][column] - KHP[row][column];
                }
            }

            makeSymmetrical();
            limitCovariance();
        }

    }
    
    void EKF::fuseMag()
    {
        // assign intermediate variables
        float q0 = _state.quat_nominal(0);
        float q1 = _state.quat_nominal(1);
        float q2 = _state.quat_nominal(2);
        float q3 = _state.quat_nominal(3);

        float magN = _state.mag_I(0);
        float magE = _state.mag_I(1);
        float magD = _state.mag_I(2);

        // XYZ Measurement uncertainty. Need to consider timing errors for fast rotations
        float R_MAG = 1e-3f;

        // intermediate variables from algebraic optimisation
        float SH_MAG[9];
        SH_MAG[0] = math::square(q0) - math::square(q1) + math::square(q2) - math::square(q3);
        SH_MAG[1] = math::square(q0) + math::square(q1) - math::square(q2) - math::square(q3);
        SH_MAG[2] = math::square(q0) - math::square(q1) - math::square(q2) + math::square(q3);
        SH_MAG[3] = 2 * q0 * q1 + 2 * q2 * q3;
        SH_MAG[4] = 2 * q0 * q3 + 2 * q1 * q2;
        SH_MAG[5] = 2 * q0 * q2 + 2 * q1 * q3;
        SH_MAG[6] = magE * (2 * q0 * q1 - 2 * q2 * q3);
        SH_MAG[7] = 2 * q1 * q3 - 2 * q0 * q2;
        SH_MAG[8] = 2 * q0 * q3;

        // rotate magnetometer earth field state into body frame
        matrix::Dcm<float> R_to_body(_state.quat_nominal);
        R_to_body = R_to_body.transpose();

        Vector3f mag_I_rot = R_to_body * _state.mag_I;

        // compute magnetometer innovations
        _mag_innov[0] = (mag_I_rot(0) + _state.mag_B(0)) - _mag_sample_delayed.mag(0);
        _mag_innov[1] = (mag_I_rot(1) + _state.mag_B(1)) - _mag_sample_delayed.mag(1);
        _mag_innov[2] = (mag_I_rot(2) + _state.mag_B(2)) - _mag_sample_delayed.mag(2);

        // Note that although the observation jacobians and kalman gains are decalred as arrays
        // sequential fusion of the X,Y and Z components is used.
        float H_MAG[3][24] = {};
        float Kfusion[24] = {};

        // Calculate observation Jacobians and kalman gains for each magentoemter axis
        // X Axis
        H_MAG[0][1] = SH_MAG[6] - magD * SH_MAG[2] - magN * SH_MAG[5];
        H_MAG[0][2] = magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2);
        H_MAG[0][16] = SH_MAG[1];
        H_MAG[0][17] = SH_MAG[4];
        H_MAG[0][18] = SH_MAG[7];
        H_MAG[0][19] = 1;

        // intermediate variables
        float SK_MX[4] = {};
        // innovation variance
        _mag_innov_var[0] = (P[19][19] + R_MAG - P[1][19] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[16][19] *
                     SH_MAG[1]
                     + P[17][19] * SH_MAG[4] + P[18][19] * SH_MAG[7] + P[2][19] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                             (SH_MAG[8] - 2 * q1 * q2)) - (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) * (P[19][1] - P[1][1] *
                                     (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[16][1] * SH_MAG[1] + P[17][1] * SH_MAG[4] + P[18][1] * SH_MAG[7] +
                                     P[2][1] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2))) + SH_MAG[1] *
                     (P[19][16] - P[1][16] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[16][16] * SH_MAG[1] + P[17][16] *
                      SH_MAG[4] + P[18][16] * SH_MAG[7] + P[2][16] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                              (SH_MAG[8] - 2 * q1 * q2))) + SH_MAG[4] * (P[19][17] - P[1][17] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) +
                                      P[16][17] * SH_MAG[1] + P[17][17] * SH_MAG[4] + P[18][17] * SH_MAG[7] + P[2][17] * (magE * SH_MAG[0] + magD * SH_MAG[3]
                                              - magN * (SH_MAG[8] - 2 * q1 * q2))) + SH_MAG[7] * (P[19][18] - P[1][18] * (magD * SH_MAG[2] - SH_MAG[6] + magN *
                                                      SH_MAG[5]) + P[16][18] * SH_MAG[1] + P[17][18] * SH_MAG[4] + P[18][18] * SH_MAG[7] + P[2][18] *
                                                      (magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2))) + (magE * SH_MAG[0] + magD * SH_MAG[3] -
                                                              magN * (SH_MAG[8] - 2 * q1 * q2)) * (P[19][2] - P[1][2] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[16][2] *
                                                                      SH_MAG[1] + P[17][2] * SH_MAG[4] + P[18][2] * SH_MAG[7] + P[2][2] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                                                                              (SH_MAG[8] - 2 * q1 * q2))));

        // check for a badly conditioned covariance matrix
        if (_mag_innov_var[0] >= R_MAG) {
            // the innovation variance contribution from the state covariances is non-negative - no fault
            _fault_status.bad_mag_x = false;

        } else {
            // the innovation variance contribution from the state covariances is negtive which means the covariance matrix is badly conditioned
            _fault_status.bad_mag_x = true;
            // we need to reinitialise the covariance matrix and abort this fusion step
            initialiseCovariance();
            return;
        }


        // Y axis

        H_MAG[1][0] = magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5];
        H_MAG[1][2] = - magE * SH_MAG[4] - magD * SH_MAG[7] - magN * SH_MAG[1];
        H_MAG[1][16] = 2 * q1 * q2 - SH_MAG[8];
        H_MAG[1][17] = SH_MAG[0];
        H_MAG[1][18] = SH_MAG[3];
        H_MAG[1][20] = 1;

        // intermediate variables - note SK_MY[0] is 1/(innovation variance)
        float SK_MY[4];
        _mag_innov_var[1] = (P[20][20] + R_MAG + P[0][20] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[17][20] *
                     SH_MAG[0]
                     + P[18][20] * SH_MAG[3] - (SH_MAG[8] - 2 * q1 * q2) * (P[20][16] + P[0][16] * (magD * SH_MAG[2] - SH_MAG[6] + magN *
                             SH_MAG[5]) + P[17][16] * SH_MAG[0] + P[18][16] * SH_MAG[3] - P[2][16] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN *
                                     SH_MAG[1]) - P[16][16] * (SH_MAG[8] - 2 * q1 * q2)) - P[2][20] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN *
                                             SH_MAG[1]) + (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) * (P[20][0] + P[0][0] *
                                                     (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[17][0] * SH_MAG[0] + P[18][0] * SH_MAG[3] - P[2][0] *
                                                     (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[16][0] * (SH_MAG[8] - 2 * q1 * q2)) + SH_MAG[0] *
                     (P[20][17] + P[0][17] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[17][17] * SH_MAG[0] + P[18][17] *
                      SH_MAG[3] - P[2][17] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[16][17] *
                      (SH_MAG[8] - 2 * q1 * q2)) + SH_MAG[3] * (P[20][18] + P[0][18] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) +
                              P[17][18] * SH_MAG[0] + P[18][18] * SH_MAG[3] - P[2][18] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) -
                              P[16][18] * (SH_MAG[8] - 2 * q1 * q2)) - P[16][20] * (SH_MAG[8] - 2 * q1 * q2) - (magE * SH_MAG[4] + magD * SH_MAG[7] +
                                      magN * SH_MAG[1]) * (P[20][2] + P[0][2] * (magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5]) + P[17][2] * SH_MAG[0] +
                                              P[18][2] * SH_MAG[3] - P[2][2] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[16][2] *
                                              (SH_MAG[8] - 2 * q1 * q2)));

        // check for a badly conditioned covariance matrix
        if (_mag_innov_var[1] >= R_MAG) {
            // the innovation variance contribution from the state covariances is non-negative - no fault
            _fault_status.bad_mag_y = false;

        } else {
            // the innovation variance contribution from the state covariances is negtive which means the covariance matrix is badly conditioned
            _fault_status.bad_mag_y = true;
            // we need to reinitialise the covariance matrix and abort this fusion step
            initialiseCovariance();
            return;
        }


        // Z axis

        H_MAG[2][0] = magN * (SH_MAG[8] - 2 * q1 * q2) - magD * SH_MAG[3] - magE * SH_MAG[0];
        H_MAG[2][1] = magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1];
        H_MAG[2][16] = SH_MAG[5];
        H_MAG[2][17] = 2 * q2 * q3 - 2 * q0 * q1;
        H_MAG[2][18] = SH_MAG[2];
        H_MAG[2][21] = 1;

        // intermediate variables
        float SK_MZ[4];
        _mag_innov_var[2] = (P[21][21] + R_MAG + P[16][21] * SH_MAG[5] + P[18][21] * SH_MAG[2] - (2 * q0 * q1 - 2 * q2 * q3) *
                     (P[21][17] + P[16][17] * SH_MAG[5] + P[18][17] * SH_MAG[2] - P[0][17] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                             (SH_MAG[8] - 2 * q1 * q2)) + P[1][17] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[17][17] *
                      (2 * q0 * q1 - 2 * q2 * q3)) - P[0][21] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                              (SH_MAG[8] - 2 * q1 * q2)) + P[1][21] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) + SH_MAG[5] *
                     (P[21][16] + P[16][16] * SH_MAG[5] + P[18][16] * SH_MAG[2] - P[0][16] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                             (SH_MAG[8] - 2 * q1 * q2)) + P[1][16] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[17][16] *
                      (2 * q0 * q1 - 2 * q2 * q3)) + SH_MAG[2] * (P[21][18] + P[16][18] * SH_MAG[5] + P[18][18] * SH_MAG[2] - P[0][18] *
                              (magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2)) + P[1][18] *
                              (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[17][18] * (2 * q0 * q1 - 2 * q2 * q3)) -
                     (magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2)) * (P[21][0] + P[16][0] * SH_MAG[5] + P[18][0] *
                             SH_MAG[2] - P[0][0] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2)) + P[1][0] *
                             (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[17][0] * (2 * q0 * q1 - 2 * q2 * q3)) - P[17][21] *
                     (2 * q0 * q1 - 2 * q2 * q3) + (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) *
                     (P[21][1] + P[16][1] * SH_MAG[5] + P[18][1] * SH_MAG[2] - P[0][1] * (magE * SH_MAG[0] + magD * SH_MAG[3] - magN *
                             (SH_MAG[8] - 2 * q1 * q2)) + P[1][1] * (magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1]) - P[17][1] *
                      (2 * q0 * q1 - 2 * q2 * q3)));

        // check for a badly conditioned covariance matrix
        if (_mag_innov_var[2] >= R_MAG) {
            // the innovation variance contribution from the state covariances is non-negative - no fault
            _fault_status.bad_mag_z = false;

        } else {
            // the innovation variance contribution from the state covariances is negtive which means the covariance matrix is badly conditioned
            _fault_status.bad_mag_z = true;
            // we need to reinitialise the covariance matrix and abort this fusion step
            initialiseCovariance();
            return;
        }


        // Perform an innovation consistency check on each measurement and if one axis fails
        // do not fuse any data from the sensor because the most common errors affect multiple axes.
        _mag_healthy = true;

        for (uint8_t index = 0; index <= 2; index++) {
            _mag_test_ratio[index] = math::square(_mag_innov[index]) / (math::square(std::max(_params.mag_innov_gate, 1.0f)) * _mag_innov_var[index]);

            if (_mag_test_ratio[index] > 1.0f) {
                _mag_healthy = false;
            }
        }

        if (!_mag_healthy) {
            return;
        }

        // update the states and covariance usinng sequential fusion of the magnetometer components
        for (uint8_t index = 0; index <= 2; index++) {
            // Calculate Kalman gains
            if (index == 0) {
                // Calculate X axis Kalman gains
                SK_MX[0] = 1.0f / _mag_innov_var[0];
                SK_MX[1] = magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2);
                SK_MX[2] = magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5];
                SK_MX[3] = SH_MAG[7];

                Kfusion[0] = SK_MX[0] * (P[0][19] + P[0][16] * SH_MAG[1] + P[0][17] * SH_MAG[4] - P[0][1] * SK_MX[2] + P[0][2] *
                             SK_MX[1] + P[0][18] * SK_MX[3]);
                Kfusion[1] = SK_MX[0] * (P[1][19] + P[1][16] * SH_MAG[1] + P[1][17] * SH_MAG[4] - P[1][1] * SK_MX[2] + P[1][2] *
                             SK_MX[1] + P[1][18] * SK_MX[3]);
                Kfusion[2] = SK_MX[0] * (P[2][19] + P[2][16] * SH_MAG[1] + P[2][17] * SH_MAG[4] - P[2][1] * SK_MX[2] + P[2][2] *
                             SK_MX[1] + P[2][18] * SK_MX[3]);
                Kfusion[3] = SK_MX[0] * (P[3][19] + P[3][16] * SH_MAG[1] + P[3][17] * SH_MAG[4] - P[3][1] * SK_MX[2] + P[3][2] *
                             SK_MX[1] + P[3][18] * SK_MX[3]);
                Kfusion[4] = SK_MX[0] * (P[4][19] + P[4][16] * SH_MAG[1] + P[4][17] * SH_MAG[4] - P[4][1] * SK_MX[2] + P[4][2] *
                             SK_MX[1] + P[4][18] * SK_MX[3]);
                Kfusion[5] = SK_MX[0] * (P[5][19] + P[5][16] * SH_MAG[1] + P[5][17] * SH_MAG[4] - P[5][1] * SK_MX[2] + P[5][2] *
                             SK_MX[1] + P[5][18] * SK_MX[3]);
                Kfusion[6] = SK_MX[0] * (P[6][19] + P[6][16] * SH_MAG[1] + P[6][17] * SH_MAG[4] - P[6][1] * SK_MX[2] + P[6][2] *
                             SK_MX[1] + P[6][18] * SK_MX[3]);
                Kfusion[7] = SK_MX[0] * (P[7][19] + P[7][16] * SH_MAG[1] + P[7][17] * SH_MAG[4] - P[7][1] * SK_MX[2] + P[7][2] *
                             SK_MX[1] + P[7][18] * SK_MX[3]);
                Kfusion[8] = SK_MX[0] * (P[8][19] + P[8][16] * SH_MAG[1] + P[8][17] * SH_MAG[4] - P[8][1] * SK_MX[2] + P[8][2] *
                             SK_MX[1] + P[8][18] * SK_MX[3]);
                Kfusion[9] = SK_MX[0] * (P[9][19] + P[9][16] * SH_MAG[1] + P[9][17] * SH_MAG[4] - P[9][1] * SK_MX[2] + P[9][2] *
                             SK_MX[1] + P[9][18] * SK_MX[3]);
                Kfusion[10] = SK_MX[0] * (P[10][19] + P[10][16] * SH_MAG[1] + P[10][17] * SH_MAG[4] - P[10][1] * SK_MX[2] + P[10][2] *
                              SK_MX[1] + P[10][18] * SK_MX[3]);
                Kfusion[11] = SK_MX[0] * (P[11][19] + P[11][16] * SH_MAG[1] + P[11][17] * SH_MAG[4] - P[11][1] * SK_MX[2] + P[11][2] *
                              SK_MX[1] + P[11][18] * SK_MX[3]);
                Kfusion[12] = SK_MX[0] * (P[12][19] + P[12][16] * SH_MAG[1] + P[12][17] * SH_MAG[4] - P[12][1] * SK_MX[2] + P[12][2] *
                              SK_MX[1] + P[12][18] * SK_MX[3]);
                Kfusion[13] = SK_MX[0] * (P[13][19] + P[13][16] * SH_MAG[1] + P[13][17] * SH_MAG[4] - P[13][1] * SK_MX[2] + P[13][2] *
                              SK_MX[1] + P[13][18] * SK_MX[3]);
                Kfusion[14] = SK_MX[0] * (P[14][19] + P[14][16] * SH_MAG[1] + P[14][17] * SH_MAG[4] - P[14][1] * SK_MX[2] + P[14][2] *
                              SK_MX[1] + P[14][18] * SK_MX[3]);
                Kfusion[15] = SK_MX[0] * (P[15][19] + P[15][16] * SH_MAG[1] + P[15][17] * SH_MAG[4] - P[15][1] * SK_MX[2] + P[15][2] *
                              SK_MX[1] + P[15][18] * SK_MX[3]);
                Kfusion[16] = SK_MX[0] * (P[16][19] + P[16][16] * SH_MAG[1] + P[16][17] * SH_MAG[4] - P[16][1] * SK_MX[2] + P[16][2] *
                              SK_MX[1] + P[16][18] * SK_MX[3]);
                Kfusion[17] = SK_MX[0] * (P[17][19] + P[17][16] * SH_MAG[1] + P[17][17] * SH_MAG[4] - P[17][1] * SK_MX[2] + P[17][2] *
                              SK_MX[1] + P[17][18] * SK_MX[3]);
                Kfusion[18] = SK_MX[0] * (P[18][19] + P[18][16] * SH_MAG[1] + P[18][17] * SH_MAG[4] - P[18][1] * SK_MX[2] + P[18][2] *
                              SK_MX[1] + P[18][18] * SK_MX[3]);
                Kfusion[19] = SK_MX[0] * (P[19][19] + P[19][16] * SH_MAG[1] + P[19][17] * SH_MAG[4] - P[19][1] * SK_MX[2] + P[19][2] *
                              SK_MX[1] + P[19][18] * SK_MX[3]);
                Kfusion[20] = SK_MX[0] * (P[20][19] + P[20][16] * SH_MAG[1] + P[20][17] * SH_MAG[4] - P[20][1] * SK_MX[2] + P[20][2] *
                              SK_MX[1] + P[20][18] * SK_MX[3]);
                Kfusion[21] = SK_MX[0] * (P[21][19] + P[21][16] * SH_MAG[1] + P[21][17] * SH_MAG[4] - P[21][1] * SK_MX[2] + P[21][2] *
                              SK_MX[1] + P[21][18] * SK_MX[3]);
                Kfusion[22] = SK_MX[0] * (P[22][19] + P[22][16] * SH_MAG[1] + P[22][17] * SH_MAG[4] - P[22][1] * SK_MX[2] + P[22][2] *
                              SK_MX[1] + P[22][18] * SK_MX[3]);
                Kfusion[23] = SK_MX[0] * (P[23][19] + P[23][16] * SH_MAG[1] + P[23][17] * SH_MAG[4] - P[23][1] * SK_MX[2] + P[23][2] *
                              SK_MX[1] + P[23][18] * SK_MX[3]);

            } else if (index == 1) {
                // Calculate Y axis Kalman gains
                SK_MY[0] = 1.0f / _mag_innov_var[1];
                SK_MY[1] = magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1];
                SK_MY[2] = magD * SH_MAG[2] - SH_MAG[6] + magN * SH_MAG[5];
                SK_MY[3] = SH_MAG[8] - 2 * q1 * q2;

                Kfusion[0] = SK_MY[0] * (P[0][20] + P[0][17] * SH_MAG[0] + P[0][18] * SH_MAG[3] + P[0][0] * SK_MY[2] - P[0][2] *
                             SK_MY[1] - P[0][16] * SK_MY[3]);
                Kfusion[1] = SK_MY[0] * (P[1][20] + P[1][17] * SH_MAG[0] + P[1][18] * SH_MAG[3] + P[1][0] * SK_MY[2] - P[1][2] *
                             SK_MY[1] - P[1][16] * SK_MY[3]);
                Kfusion[2] = SK_MY[0] * (P[2][20] + P[2][17] * SH_MAG[0] + P[2][18] * SH_MAG[3] + P[2][0] * SK_MY[2] - P[2][2] *
                             SK_MY[1] - P[2][16] * SK_MY[3]);
                Kfusion[3] = SK_MY[0] * (P[3][20] + P[3][17] * SH_MAG[0] + P[3][18] * SH_MAG[3] + P[3][0] * SK_MY[2] - P[3][2] *
                             SK_MY[1] - P[3][16] * SK_MY[3]);
                Kfusion[4] = SK_MY[0] * (P[4][20] + P[4][17] * SH_MAG[0] + P[4][18] * SH_MAG[3] + P[4][0] * SK_MY[2] - P[4][2] *
                             SK_MY[1] - P[4][16] * SK_MY[3]);
                Kfusion[5] = SK_MY[0] * (P[5][20] + P[5][17] * SH_MAG[0] + P[5][18] * SH_MAG[3] + P[5][0] * SK_MY[2] - P[5][2] *
                             SK_MY[1] - P[5][16] * SK_MY[3]);
                Kfusion[6] = SK_MY[0] * (P[6][20] + P[6][17] * SH_MAG[0] + P[6][18] * SH_MAG[3] + P[6][0] * SK_MY[2] - P[6][2] *
                             SK_MY[1] - P[6][16] * SK_MY[3]);
                Kfusion[7] = SK_MY[0] * (P[7][20] + P[7][17] * SH_MAG[0] + P[7][18] * SH_MAG[3] + P[7][0] * SK_MY[2] - P[7][2] *
                             SK_MY[1] - P[7][16] * SK_MY[3]);
                Kfusion[8] = SK_MY[0] * (P[8][20] + P[8][17] * SH_MAG[0] + P[8][18] * SH_MAG[3] + P[8][0] * SK_MY[2] - P[8][2] *
                             SK_MY[1] - P[8][16] * SK_MY[3]);
                Kfusion[9] = SK_MY[0] * (P[9][20] + P[9][17] * SH_MAG[0] + P[9][18] * SH_MAG[3] + P[9][0] * SK_MY[2] - P[9][2] *
                             SK_MY[1] - P[9][16] * SK_MY[3]);
                Kfusion[10] = SK_MY[0] * (P[10][20] + P[10][17] * SH_MAG[0] + P[10][18] * SH_MAG[3] + P[10][0] * SK_MY[2] - P[10][2] *
                              SK_MY[1] - P[10][16] * SK_MY[3]);
                Kfusion[11] = SK_MY[0] * (P[11][20] + P[11][17] * SH_MAG[0] + P[11][18] * SH_MAG[3] + P[11][0] * SK_MY[2] - P[11][2] *
                              SK_MY[1] - P[11][16] * SK_MY[3]);
                Kfusion[12] = SK_MY[0] * (P[12][20] + P[12][17] * SH_MAG[0] + P[12][18] * SH_MAG[3] + P[12][0] * SK_MY[2] - P[12][2] *
                              SK_MY[1] - P[12][16] * SK_MY[3]);
                Kfusion[13] = SK_MY[0] * (P[13][20] + P[13][17] * SH_MAG[0] + P[13][18] * SH_MAG[3] + P[13][0] * SK_MY[2] - P[13][2] *
                              SK_MY[1] - P[13][16] * SK_MY[3]);
                Kfusion[14] = SK_MY[0] * (P[14][20] + P[14][17] * SH_MAG[0] + P[14][18] * SH_MAG[3] + P[14][0] * SK_MY[2] - P[14][2] *
                              SK_MY[1] - P[14][16] * SK_MY[3]);
                Kfusion[15] = SK_MY[0] * (P[15][20] + P[15][17] * SH_MAG[0] + P[15][18] * SH_MAG[3] + P[15][0] * SK_MY[2] - P[15][2] *
                              SK_MY[1] - P[15][16] * SK_MY[3]);
                Kfusion[16] = SK_MY[0] * (P[16][20] + P[16][17] * SH_MAG[0] + P[16][18] * SH_MAG[3] + P[16][0] * SK_MY[2] - P[16][2] *
                              SK_MY[1] - P[16][16] * SK_MY[3]);
                Kfusion[17] = SK_MY[0] * (P[17][20] + P[17][17] * SH_MAG[0] + P[17][18] * SH_MAG[3] + P[17][0] * SK_MY[2] - P[17][2] *
                              SK_MY[1] - P[17][16] * SK_MY[3]);
                Kfusion[18] = SK_MY[0] * (P[18][20] + P[18][17] * SH_MAG[0] + P[18][18] * SH_MAG[3] + P[18][0] * SK_MY[2] - P[18][2] *
                              SK_MY[1] - P[18][16] * SK_MY[3]);
                Kfusion[19] = SK_MY[0] * (P[19][20] + P[19][17] * SH_MAG[0] + P[19][18] * SH_MAG[3] + P[19][0] * SK_MY[2] - P[19][2] *
                              SK_MY[1] - P[19][16] * SK_MY[3]);
                Kfusion[20] = SK_MY[0] * (P[20][20] + P[20][17] * SH_MAG[0] + P[20][18] * SH_MAG[3] + P[20][0] * SK_MY[2] - P[20][2] *
                              SK_MY[1] - P[20][16] * SK_MY[3]);
                Kfusion[21] = SK_MY[0] * (P[21][20] + P[21][17] * SH_MAG[0] + P[21][18] * SH_MAG[3] + P[21][0] * SK_MY[2] - P[21][2] *
                              SK_MY[1] - P[21][16] * SK_MY[3]);
                Kfusion[22] = SK_MY[0] * (P[22][20] + P[22][17] * SH_MAG[0] + P[22][18] * SH_MAG[3] + P[22][0] * SK_MY[2] - P[22][2] *
                              SK_MY[1] - P[22][16] * SK_MY[3]);
                Kfusion[23] = SK_MY[0] * (P[23][20] + P[23][17] * SH_MAG[0] + P[23][18] * SH_MAG[3] + P[23][0] * SK_MY[2] - P[23][2] *
                              SK_MY[1] - P[23][16] * SK_MY[3]);

            } else if (index == 2) {
                // Calculate Z axis Kalman gains
                SK_MZ[0] = 1.0f / _mag_innov_var[2];
                SK_MZ[1] = magE * SH_MAG[0] + magD * SH_MAG[3] - magN * (SH_MAG[8] - 2 * q1 * q2);
                SK_MZ[2] = magE * SH_MAG[4] + magD * SH_MAG[7] + magN * SH_MAG[1];
                SK_MZ[3] = 2 * q0 * q1 - 2 * q2 * q3;

                Kfusion[0] = SK_MZ[0] * (P[0][21] + P[0][18] * SH_MAG[2] + P[0][16] * SH_MAG[5] - P[0][0] * SK_MZ[1] + P[0][1] *
                             SK_MZ[2] - P[0][17] * SK_MZ[3]);
                Kfusion[1] = SK_MZ[0] * (P[1][21] + P[1][18] * SH_MAG[2] + P[1][16] * SH_MAG[5] - P[1][0] * SK_MZ[1] + P[1][1] *
                             SK_MZ[2] - P[1][17] * SK_MZ[3]);
                Kfusion[2] = SK_MZ[0] * (P[2][21] + P[2][18] * SH_MAG[2] + P[2][16] * SH_MAG[5] - P[2][0] * SK_MZ[1] + P[2][1] *
                             SK_MZ[2] - P[2][17] * SK_MZ[3]);
                Kfusion[3] = SK_MZ[0] * (P[3][21] + P[3][18] * SH_MAG[2] + P[3][16] * SH_MAG[5] - P[3][0] * SK_MZ[1] + P[3][1] *
                             SK_MZ[2] - P[3][17] * SK_MZ[3]);
                Kfusion[4] = SK_MZ[0] * (P[4][21] + P[4][18] * SH_MAG[2] + P[4][16] * SH_MAG[5] - P[4][0] * SK_MZ[1] + P[4][1] *
                             SK_MZ[2] - P[4][17] * SK_MZ[3]);
                Kfusion[5] = SK_MZ[0] * (P[5][21] + P[5][18] * SH_MAG[2] + P[5][16] * SH_MAG[5] - P[5][0] * SK_MZ[1] + P[5][1] *
                             SK_MZ[2] - P[5][17] * SK_MZ[3]);
                Kfusion[6] = SK_MZ[0] * (P[6][21] + P[6][18] * SH_MAG[2] + P[6][16] * SH_MAG[5] - P[6][0] * SK_MZ[1] + P[6][1] *
                             SK_MZ[2] - P[6][17] * SK_MZ[3]);
                Kfusion[7] = SK_MZ[0] * (P[7][21] + P[7][18] * SH_MAG[2] + P[7][16] * SH_MAG[5] - P[7][0] * SK_MZ[1] + P[7][1] *
                             SK_MZ[2] - P[7][17] * SK_MZ[3]);
                Kfusion[8] = SK_MZ[0] * (P[8][21] + P[8][18] * SH_MAG[2] + P[8][16] * SH_MAG[5] - P[8][0] * SK_MZ[1] + P[8][1] *
                             SK_MZ[2] - P[8][17] * SK_MZ[3]);
                Kfusion[9] = SK_MZ[0] * (P[9][21] + P[9][18] * SH_MAG[2] + P[9][16] * SH_MAG[5] - P[9][0] * SK_MZ[1] + P[9][1] *
                             SK_MZ[2] - P[9][17] * SK_MZ[3]);
                Kfusion[10] = SK_MZ[0] * (P[10][21] + P[10][18] * SH_MAG[2] + P[10][16] * SH_MAG[5] - P[10][0] * SK_MZ[1] + P[10][1] *
                              SK_MZ[2] - P[10][17] * SK_MZ[3]);
                Kfusion[11] = SK_MZ[0] * (P[11][21] + P[11][18] * SH_MAG[2] + P[11][16] * SH_MAG[5] - P[11][0] * SK_MZ[1] + P[11][1] *
                              SK_MZ[2] - P[11][17] * SK_MZ[3]);
                Kfusion[12] = SK_MZ[0] * (P[12][21] + P[12][18] * SH_MAG[2] + P[12][16] * SH_MAG[5] - P[12][0] * SK_MZ[1] + P[12][1] *
                              SK_MZ[2] - P[12][17] * SK_MZ[3]);
                Kfusion[13] = SK_MZ[0] * (P[13][21] + P[13][18] * SH_MAG[2] + P[13][16] * SH_MAG[5] - P[13][0] * SK_MZ[1] + P[13][1] *
                              SK_MZ[2] - P[13][17] * SK_MZ[3]);
                Kfusion[14] = SK_MZ[0] * (P[14][21] + P[14][18] * SH_MAG[2] + P[14][16] * SH_MAG[5] - P[14][0] * SK_MZ[1] + P[14][1] *
                              SK_MZ[2] - P[14][17] * SK_MZ[3]);
                Kfusion[15] = SK_MZ[0] * (P[15][21] + P[15][18] * SH_MAG[2] + P[15][16] * SH_MAG[5] - P[15][0] * SK_MZ[1] + P[15][1] *
                              SK_MZ[2] - P[15][17] * SK_MZ[3]);
                Kfusion[16] = SK_MZ[0] * (P[16][21] + P[16][18] * SH_MAG[2] + P[16][16] * SH_MAG[5] - P[16][0] * SK_MZ[1] + P[16][1] *
                              SK_MZ[2] - P[16][17] * SK_MZ[3]);
                Kfusion[17] = SK_MZ[0] * (P[17][21] + P[17][18] * SH_MAG[2] + P[17][16] * SH_MAG[5] - P[17][0] * SK_MZ[1] + P[17][1] *
                              SK_MZ[2] - P[17][17] * SK_MZ[3]);
                Kfusion[18] = SK_MZ[0] * (P[18][21] + P[18][18] * SH_MAG[2] + P[18][16] * SH_MAG[5] - P[18][0] * SK_MZ[1] + P[18][1] *
                              SK_MZ[2] - P[18][17] * SK_MZ[3]);
                Kfusion[19] = SK_MZ[0] * (P[19][21] + P[19][18] * SH_MAG[2] + P[19][16] * SH_MAG[5] - P[19][0] * SK_MZ[1] + P[19][1] *
                              SK_MZ[2] - P[19][17] * SK_MZ[3]);
                Kfusion[20] = SK_MZ[0] * (P[20][21] + P[20][18] * SH_MAG[2] + P[20][16] * SH_MAG[5] - P[20][0] * SK_MZ[1] + P[20][1] *
                              SK_MZ[2] - P[20][17] * SK_MZ[3]);
                Kfusion[21] = SK_MZ[0] * (P[21][21] + P[21][18] * SH_MAG[2] + P[21][16] * SH_MAG[5] - P[21][0] * SK_MZ[1] + P[21][1] *
                              SK_MZ[2] - P[21][17] * SK_MZ[3]);
                Kfusion[22] = SK_MZ[0] * (P[22][21] + P[22][18] * SH_MAG[2] + P[22][16] * SH_MAG[5] - P[22][0] * SK_MZ[1] + P[22][1] *
                              SK_MZ[2] - P[22][17] * SK_MZ[3]);
                Kfusion[23] = SK_MZ[0] * (P[23][21] + P[23][18] * SH_MAG[2] + P[23][16] * SH_MAG[5] - P[23][0] * SK_MZ[1] + P[23][1] *
                              SK_MZ[2] - P[23][17] * SK_MZ[3]);

            } else {
                return;
            }

            // by definition our error state is zero at the time of fusion
            _state.ang_error.setZero();

            fuse(Kfusion, _mag_innov[index]);

            Quaternion q_correction;
            q_correction.from_axis_angle(_state.ang_error);
            _state.quat_nominal = q_correction * _state.quat_nominal;
            _state.quat_nominal.normalize();
            _state.ang_error.setZero();

            // apply covariance correction via P_new = (I -K*H)*P
            // first calculate expression for KHP
            // then calculate P - KHP
            float KH[_k_num_states][_k_num_states] = {};

            for (unsigned row = 0; row < _k_num_states; row++) {
                for (unsigned column = 0; column < 3; column++) {
                    KH[row][column] = Kfusion[row] * H_MAG[index][column];
                }

                for (unsigned column = 16; column < 22; column++) {
                    KH[row][column] = Kfusion[row] * H_MAG[index][column];
                }

            }

            float KHP[_k_num_states][_k_num_states] = {};

            for (unsigned row = 0; row < _k_num_states; row++) {
                for (unsigned column = 0; column < _k_num_states; column++) {
                    float tmp = KH[row][0] * P[0][column];
                    tmp += KH[row][1] * P[1][column];
                    tmp += KH[row][2] * P[2][column];
                    tmp += KH[row][16] * P[16][column];
                    tmp += KH[row][17] * P[17][column];
                    tmp += KH[row][18] * P[18][column];
                    tmp += KH[row][19] * P[19][column];
                    tmp += KH[row][20] * P[20][column];
                    tmp += KH[row][21] * P[21][column];
                    KHP[row][column] = tmp;
                }
            }

            for (unsigned row = 0; row < _k_num_states; row++) {
                for (unsigned column = 0; column < _k_num_states; column++) {
                    P[row][column] -= KHP[row][column];
                }
            }

            makeSymmetrical();
            limitCovariance();
        }
    }
    
    void EKF::fuseHeading()
    {
        // assign intermediate state variables
        float q0 = _state.quat_nominal(0);
        float q1 = _state.quat_nominal(1);
        float q2 = _state.quat_nominal(2);
        float q3 = _state.quat_nominal(3);

        float magX = _mag_sample_delayed.mag(0);
        float magY = _mag_sample_delayed.mag(1);
        float magZ = _mag_sample_delayed.mag(2);

        float R_mag = _params.mag_heading_noise;

        float t2 = q0 * q0;
        float t3 = q1 * q1;
        float t4 = q2 * q2;
        float t5 = q3 * q3;
        float t6 = q0 * q2 * 2.0f;
        float t7 = q1 * q3 * 2.0f;
        float t8 = t6 + t7;
        float t9 = q0 * q3 * 2.0f;
        float t13 = q1 * q2 * 2.0f;
        float t10 = t9 - t13;
        float t11 = t2 + t3 - t4 - t5;
        float t12 = magX * t11;
        float t14 = magZ * t8;
        float t19 = magY * t10;
        float t15 = t12 + t14 - t19;
        float t16 = t2 - t3 + t4 - t5;
        float t17 = q0 * q1 * 2.0f;
        float t24 = q2 * q3 * 2.0f;
        float t18 = t17 - t24;
        float t20 = 1.0f / t15;
        float t21 = magY * t16;
        float t22 = t9 + t13;
        float t23 = magX * t22;
        float t28 = magZ * t18;
        float t25 = t21 + t23 - t28;
        float t29 = t20 * t25;
        float t26 = tan(t29);
        float t27 = 1.0f / (t15 * t15);
        float t30 = t26 * t26;
        float t31 = t30 + 1.0f;

        float H_MAG[3] = {};
        H_MAG[0] = -t31 * (t20 * (magZ * t16 + magY * t18) + t25 * t27 * (magY * t8 + magZ * t10));
        H_MAG[1] = t31 * (t20 * (magX * t18 + magZ * t22) + t25 * t27 * (magX * t8 - magZ * t11));
        H_MAG[2] = t31 * (t20 * (magX * t16 - magY * t22) + t25 * t27 * (magX * t10 + magY * t11));

        // calculate innovation
        matrix::Dcm<float> R_to_earth(_state.quat_nominal);
        matrix::Vector3f mag_earth_pred = R_to_earth * _mag_sample_delayed.mag;

        float innovation = atan2f(mag_earth_pred(1), mag_earth_pred(0)) - math::radians(_params.mag_declination_deg);

        innovation = math::constrain(innovation, -0.5f, 0.5f);
        _heading_innov = innovation;

        float innovation_var = R_mag;
        _heading_innov_var = innovation_var;

        // calculate innovation variance
        float PH[3] = {};

        for (unsigned row = 0; row < 3; row++) {
            for (unsigned column = 0; column < 3; column++) {
                PH[row] += P[row][column] * H_MAG[column];
            }

            innovation_var += H_MAG[row] * PH[row];
        }

        if (innovation_var >= R_mag) {
            // the innovation variance contribution from the state covariances is not negative, no fault
            _fault_status.bad_mag_x = false;
            _fault_status.bad_mag_y = false;
            _fault_status.bad_mag_z = false;

        } else {
            // the innovation variance contribution from the state covariances is negtive which means the covariance matrix is badly conditioned
            _fault_status.bad_mag_x = true;
            _fault_status.bad_mag_y = true;
            _fault_status.bad_mag_z = true;

            // we reinitialise the covariance matrix and abort this fusion step
            initialiseCovariance();
            return;
        }

        float innovation_var_inv = 1 / innovation_var;

        // calculate kalman gain
        float Kfusion[_k_num_states] = {};

        for (unsigned row = 0; row < _k_num_states; row++) {
            for (unsigned column = 0; column < 3; column++) {
                Kfusion[row] += P[row][column] * H_MAG[column];
            }

            Kfusion[row] *= innovation_var_inv;
        }

        // innovation test ratio
        _yaw_test_ratio = math::square(innovation) / (math::square(std::max(_params.heading_innov_gate, 1.0f)) * innovation_var);

        // set the magnetometer unhealthy if the test fails
        if (_yaw_test_ratio > 1.0f) {
            _mag_healthy = false;

            // if we are in air we don't want to fuse the measurement
            // we allow to use it when on the ground because the large innovation could be caused
            // by interference or a large initial gyro bias
            if (_control_status.flags.in_air) {
                return;
            }

        } else {
            _mag_healthy = true;
        }

        _state.ang_error.setZero();
        fuse(Kfusion, innovation);

        // correct the nominal quaternion
        Quaternion dq;
        dq.from_axis_angle(_state.ang_error);
        _state.quat_nominal = dq * _state.quat_nominal;
        _state.quat_nominal.normalize();

        float HP[_k_num_states] = {};

        for (unsigned column = 0; column < _k_num_states; column++) {
            for (unsigned row = 0; row < 3; row++) {
                HP[column] += H_MAG[row] * P[row][column];
            }
        }

        for (unsigned row = 0; row < _k_num_states; row++) {
            for (unsigned column = 0; column < _k_num_states; column++) {
                P[row][column] -= Kfusion[row] * HP[column];
            }
        }

        makeSymmetrical();
        limitCovariance();
    }

    
    void EKF::resetVelocity()
    {
        GPSSample gps_newest = _gps_buffer.newest();
        // if we have a valid GPS measurement use it to initialise velocity states
        if (!_gps_buffer.empty() && _time_last_imu - gps_newest.time_us < 100000) {
            _state.vel = gps_newest.vel;

        } else {
            _state.vel.setZero();
        }
    }
    
    void EKF::resetPosition()
    {
        // if we have a valid GPS measurement use it to initialise position states
        GPSSample gps_newest = _gps_buffer.newest();
        if (!_gps_buffer.empty() && _time_last_imu - gps_newest.time_us < 100000) {
            _state.pos(0) = gps_newest.pos(0);
            _state.pos(1) = gps_newest.pos(1);

        } else {
            // XXX use the value of the last known position
        }

        BaroSample baro_newest = _baro_buffer.newest();
        _state.pos(2) = -baro_newest.hgt;
    }

    
    bool EKF::init(uint64_t time_us)
    {
        bool ret = EKFIf::init(time_us);
        
        _state.ang_error.setZero();
        _state.vel.setZero();
        _state.pos.setZero();
        _state.gyro_bias.setZero();
        _state.gyro_scale(0) = 1.0f;
        _state.gyro_scale(1) = 1.0f;
        _state.gyro_scale(2) = 1.0f;
        _state.accel_z_bias = 0.0f;
        _state.mag_I.setZero();
        _state.mag_B.setZero();
        _state.wind_vel.setZero();
        _state.quat_nominal.setZero();
        _state.quat_nominal(0) = 1.0f;

        _output_new.vel.setZero();
        _output_new.pos.setZero();
        _output_new.quat_nominal = matrix::Quaternion<float>();


        _imu_down_sampled.delta_ang.setZero();
        _imu_down_sampled.delta_vel.setZero();
        _imu_down_sampled.delta_ang_dt = 0.0f;
        _imu_down_sampled.delta_vel_dt = 0.0f;
        _imu_down_sampled.time_us = time_us;

        _q_down_sampled(0) = 1.0f;
        _q_down_sampled(1) = 0.0f;
        _q_down_sampled(2) = 0.0f;
        _q_down_sampled(3) = 0.0f;

        _imu_updated = false;
        _NED_origin_initialised = false;
        _gps_speed_valid = false;
        _mag_healthy = false;
        return ret;
    }
    
    bool EKF::update()
    {
        if(!EKFIf::update())
            return false;
        
        bool ret = false;	// indicates if there has been an update

        if (!_filter_initialised) {
            _filter_initialised = initialiseFilter();

            if (!_filter_initialised) {
                return false;
            }
        }

        //printStates();
        //printStatesFast();
        // prediction
        if (_imu_updated) {
            ret = true;
            predictStates();
            predictCovariance();
        }

        // control logic
        controlFusionModes();

        // measurement updates

        // Fuse magnetometer data using the selected fuson method and only if angular alignment is complete
        if (_mag_buffer.popOlderThan(_imu_sample_delayed.time_us, _mag_sample_delayed)) {
            if (_control_status.flags.mag_3D && _control_status.flags.angle_align) {
                fuseMag();

                if (_control_status.flags.mag_dec) {
                    // TODO need to fuse synthetic declination measurements if there is no GPS or equivalent aiding
                    // otherwise heading will slowly drift
                }
            } else if (_control_status.flags.mag_hdg && _control_status.flags.angle_align) {
                fuseHeading();
            }
        }

        if (_baro_buffer.popOlderThan(_imu_sample_delayed.time_us, _baro_sample_delayed)) {
            _fuse_height = true;
        }

        // If we are using GPS aiding and data has fallen behind the fusion time horizon then fuse it
        // if we aren't doing any aiding, fake GPS measurements at the last known position to constrain drift
        // Coincide fake measurements with baro data for efficiency with a minimum fusion rate of 5Hz
        if (_gps_buffer.popOlderThan(_imu_sample_delayed.time_us, _gps_sample_delayed) && _control_status.flags.gps) {
            _fuse_pos = true;
            _fuse_vert_vel = true;
            _fuse_hor_vel = true;

        } else if (!_control_status.flags.gps && !_control_status.flags.opt_flow
               && ((_time_last_imu - _time_last_fake_gps > 2e5) || _fuse_height)) {
            _fuse_pos = true;
            _gps_sample_delayed.pos(0) = _last_known_posNE(0);
            _gps_sample_delayed.pos(1) = _last_known_posNE(1);
            _time_last_fake_gps = _time_last_imu;
        }

        if (_fuse_height || _fuse_pos || _fuse_hor_vel || _fuse_vert_vel) {
            fuseVelPosHeight();
            _fuse_hor_vel = _fuse_vert_vel = _fuse_pos = _fuse_height = false;
        }
	    
        calculateOutputStates();
        return ret;
    }
    
    bool EKF::collectIMU(IMUSample &imu)
    {
        imu.delta_ang(0) = imu.delta_ang(0) * _state.gyro_scale(0);
        imu.delta_ang(1) = imu.delta_ang(1) * _state.gyro_scale(1);
        imu.delta_ang(2) = imu.delta_ang(2) * _state.gyro_scale(2);

        imu.delta_ang -= _state.gyro_bias * imu.delta_ang_dt / (_dt_imu_avg > 0 ? _dt_imu_avg : 0.01f);
        imu.delta_vel(2) -= _state.accel_z_bias * imu.delta_vel_dt / (_dt_imu_avg > 0 ? _dt_imu_avg : 0.01f);;

        // store the new sample for the complementary filter prediciton
        _imu_sample_new.delta_ang = imu.delta_ang;
        _imu_sample_new.delta_vel = imu.delta_vel;
        _imu_sample_new.delta_ang_dt = imu.delta_ang_dt;
        _imu_sample_new.delta_vel_dt = imu.delta_vel_dt;
        _imu_sample_new.time_us	= imu.time_us;
        
        _imu_down_sampled.delta_ang_dt += imu.delta_ang_dt;
        _imu_down_sampled.delta_vel_dt += imu.delta_vel_dt;


        Quaternion delta_q;
        delta_q.rotate(imu.delta_ang);
        _q_down_sampled =  _q_down_sampled * delta_q;
        _q_down_sampled.normalize();

        matrix::Dcm<float> delta_R(delta_q.inversed());
        _imu_down_sampled.delta_vel = delta_R * _imu_down_sampled.delta_vel;
        _imu_down_sampled.delta_vel += imu.delta_vel;

        if ((_dt_imu_avg * _imu_ticks >= (float)(FILTER_UPDATE_PERRIOD_MS) / 1000) || 
            _dt_imu_avg * _imu_ticks >= 0.02f){
            imu.delta_ang	= _q_down_sampled.to_axis_angle();
            imu.delta_vel	= _imu_down_sampled.delta_vel;
            imu.delta_ang_dt	= _imu_down_sampled.delta_ang_dt;
            imu.delta_vel_dt	= _imu_down_sampled.delta_vel_dt;
            imu.time_us	= imu.time_us;
            _imu_down_sampled.delta_ang.setZero();
            _imu_down_sampled.delta_vel.setZero();
            _imu_down_sampled.delta_ang_dt = 0.0f;
            _imu_down_sampled.delta_vel_dt = 0.0f;
            _q_down_sampled(0) = 1.0f;
            _q_down_sampled(1) = _q_down_sampled(2) = _q_down_sampled(3) = 0.0f;
            return true;
        }

        return false;
    }

} //namespace ekf2
} //namespace control
} //namespace uquad