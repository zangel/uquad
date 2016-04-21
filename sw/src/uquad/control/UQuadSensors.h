#ifndef UQUAD_SENSORS_H
#define UQUAD_SENSORS_H

#include "Config.h"
#include "../hal/UQuad.h"
#include "Accelerometer.h"
#include "Gyroscope.h"
#include "Magnetometer.h"
#include "Barometer.h"
#include "Clock.h"


namespace uquad
{
namespace control
{
    class UQuadSensors
        : public basic_lockable_adapter<fast_mutex>
        , public Clock
        , public Accelerometer
        , public Gyroscope
        , public Magnetometer
        , public Barometer
    {
    private:
        typedef accumulators::accumulator_set
        <
            float,
            accumulators::stats
            <
                accumulators::tag::rolling_mean,
                accumulators::tag::rolling_variance
            >
        > stats_t;
        
    public:
        UQuadSensors();
        ~UQuadSensors();
        
        bool isValid() const;
        
        void setSensorsData(hal::UQuadSensorsData const &sd);
        void setAccelerometerCalibration(Transform3af const &t);
        void setGyroscopeCalibration(Transform3af const &t);
        void setMagnetometerCalibration(Transform3af const &t);
        
    protected:
        void setSensorsData(hal::UQuadSensorsData const &sd, strict_lock<UQuadSensors>&);
        void setAccelerometerCalibration(Transform3af const &t, strict_lock<UQuadSensors>&);
        void setGyroscopeCalibration(Transform3af const &t, strict_lock<UQuadSensors>&);
        void setMagnetometerCalibration(Transform3af const &t, strict_lock<UQuadSensors>&);
        
        
        system::error_code prepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
        void resetSensorData();
    
    protected:
        hal::UQuadSensorsData m_SensorData;
        base::TimePoint m_FirstTimeStamp;
        base::TimePoint m_LastTimeStamp;
        
        Transform3af m_AccelerometerCalibration;
        Transform3af m_GyroscopeCalibration;
        Transform3af m_MagnetometerCalibration;
        
        std::size_t m_BarometerStatsWS;
        Pressure m_BarometerStatsSD;
        
        stats_t m_BarometerPressureStats;
        bool m_BarometerGroundPressureValid;
        Pressure m_BarometerGroundPressure;
        
        stats_t m_BarometerTemperatureStats;
        bool m_BarometerGroundTemperatureValid;
        Temperature m_BarometerGroundTemperature;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_SENSORS_H
