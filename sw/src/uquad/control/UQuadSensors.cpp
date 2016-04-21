#include "UQuadSensors.h"

namespace uquad
{
namespace control
{
    UQuadSensors::UQuadSensors()
        : Block()
        , Clock()
        , Accelerometer()
        , Gyroscope()
        , Magnetometer()
        , Barometer()
        , m_AccelerometerCalibration(Transform3af::Identity())
        , m_GyroscopeCalibration(Transform3af::Identity())
        , m_MagnetometerCalibration(Transform3af::Identity())
        , m_BarometerStatsWS(100)
        , m_BarometerStatsSD(30.0f)
        , m_BarometerPressureStats(accumulators::tag::rolling_window::window_size = m_BarometerStatsWS)
        , m_BarometerGroundPressureValid(false)
        , m_BarometerGroundPressure(0.0f)
        , m_BarometerTemperatureStats(accumulators::tag::rolling_window::window_size = m_BarometerStatsWS)
        , m_BarometerGroundTemperatureValid(false)
        , m_BarometerGroundTemperature(0.0f)
    {
        resetSensorData();
    }
    
    UQuadSensors::~UQuadSensors()
    {
    }
    
    bool UQuadSensors::isValid() const
    {
        return true;
    }
    
    void UQuadSensors::setSensorsData(hal::UQuadSensorsData const &sd)
    {
        strict_lock<UQuadSensors> guard(*this);
        setSensorsData(sd, guard);
    }
    
    void UQuadSensors::setAccelerometerCalibration(Transform3af const &t)
    {
        strict_lock<UQuadSensors> guard(*this);
        setAccelerometerCalibration(t, guard);
    }
    
    void UQuadSensors::setGyroscopeCalibration(Transform3af const &t)
    {
        strict_lock<UQuadSensors> guard(*this);
        setGyroscopeCalibration(t, guard);
    }
    
    void UQuadSensors::setMagnetometerCalibration(Transform3af const &t)
    {
        strict_lock<UQuadSensors> guard(*this);
        setMagnetometerCalibration(t, guard);
    }
    
    void UQuadSensors::setSensorsData(hal::UQuadSensorsData const &sd, strict_lock<UQuadSensors>&)
    {
        m_SensorData = sd;
    }
    
    void UQuadSensors::setAccelerometerCalibration(Transform3af const &t, strict_lock<UQuadSensors>&)
    {
        m_AccelerometerCalibration = t;
    }
    
    void UQuadSensors::setGyroscopeCalibration(Transform3af const &t, strict_lock<UQuadSensors>&)
    {
        m_GyroscopeCalibration = t;
    }
    
    void UQuadSensors::setMagnetometerCalibration(Transform3af const &t, strict_lock<UQuadSensors>&)
    {
        m_MagnetometerCalibration = t;
    }
    
    void UQuadSensors::resetSensorData()
    {
        m_FirstTimeStamp = base::TimePoint::min();
        m_LastTimeStamp = base::TimePoint::min();
        
        m_SensorData.time = base::TimePoint::min();
        m_SensorData.velocityRate = Vec3f::Zero();
        m_SensorData.rotationRate = Vec3f::Zero();
        m_SensorData.magneticField = Vec3f::Zero();
        m_SensorData.baroTemperature = 0.0f;
        m_SensorData.baroPressure = 0.0f;
    }
    
    system::error_code UQuadSensors::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        strict_lock<UQuadSensors> guard(*this);
        resetSensorData();
        
        m_BarometerPressureStats = stats_t(accumulators::tag::rolling_window::window_size = m_BarometerStatsWS);
        m_BarometerTemperatureStats = stats_t(accumulators::tag::rolling_window::window_size = m_BarometerStatsWS);
        
        m_BarometerGroundPressureValid = false;
        m_BarometerGroundTemperatureValid = false;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code UQuadSensors::update(asio::yield_context yctx)
    {
        strict_lock<UQuadSensors> guard(*this);
        
        if(m_SensorData.time == base::TimePoint::min())
            return base::makeErrorCode(base::kEInvalidState);
        
        if(m_FirstTimeStamp == base::TimePoint::min())
        {
            m_FirstTimeStamp = m_SensorData.time;
            m_LastTimeStamp = m_SensorData.time;
        }
        
        base::TimeDuration const absT = m_SensorData.time - m_FirstTimeStamp;
        base::TimeDuration const dT = m_SensorData.time - m_LastTimeStamp;
        
        m_LastTimeStamp = m_SensorData.time;
        
        m_Time.m_Value = chrono::duration_cast<chrono::microseconds>(absT).count() * 1.0e-6f;
        m_DT.m_Value = chrono::duration_cast<chrono::microseconds>(dT).count() * 1.0e-6f;
        
        m_VelocityRate.m_Value = m_AccelerometerCalibration * m_SensorData.velocityRate;
        m_RotationRate.m_Value = m_GyroscopeCalibration * m_SensorData.rotationRate;
        m_MagneticField.m_Value = m_MagnetometerCalibration * m_SensorData.magneticField;
        
        if(!m_BarometerGroundPressureValid)
        {
            m_BarometerPressureStats(m_SensorData.baroPressure);
            m_BarometerGroundPressure = accumulators::rolling_mean(m_BarometerPressureStats);
            if(accumulators::rolling_count(m_BarometerPressureStats) >= m_BarometerStatsWS)
            {
                float const barometerPressureSD = std::sqrt(accumulators::rolling_variance(m_BarometerPressureStats));
                if(barometerPressureSD <= m_BarometerStatsSD)
                {
                    m_BarometerGroundPressureValid = true;
                }
            }
        }
        
        if(!m_BarometerGroundTemperatureValid)
        {
            m_BarometerTemperatureStats(m_SensorData.baroTemperature);
            m_BarometerGroundTemperature = accumulators::rolling_mean(m_BarometerTemperatureStats);
            if(accumulators::rolling_count(m_BarometerTemperatureStats) >= m_BarometerStatsWS)
            {
                float const barometerTemperatureSD = std::sqrt(accumulators::rolling_variance(m_BarometerTemperatureStats));
                if(barometerTemperatureSD <= m_BarometerStatsSD)
                {
                    m_BarometerGroundTemperatureValid = true;
                }
            }
        }
        
        m_Pressure.m_Value = m_SensorData.baroPressure;
        
        float baroTemperature = m_BarometerGroundTemperature + 273.15f;
        float baroPressureScaling = m_SensorData.baroPressure / m_BarometerGroundPressure;
        
        m_RelativeAltitude.m_Value = 153.8462f * baroTemperature * (1.0f - std::exp(0.190259f * std::log(baroPressureScaling)));
        m_Temperature.m_Value = m_SensorData.baroTemperature;
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad
