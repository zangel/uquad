#include "GyroscopeCalibration.h"
#include "../utility/EigenMatrixSerialization.h"

#include <cmath>

namespace uquad
{
namespace control
{
        GyroscopeCalibration::GyroscopeCalibration()
        : m_StatsWindowSize(5)
        , m_StatsSD(5.0e-3f)
        , m_StatsX(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsY(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsZ(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_MeanSample(Vec3f::Zero())
        , m_SamplesQuality(0.0f)
        , m_MapTransform(Transform3af::Identity())
        , m_CalibrationTransform(Transform3af::Identity())
    {
    }
    
    GyroscopeCalibration::~GyroscopeCalibration()
    {
    }
    
    void GyroscopeCalibration::setStatsWindowSize(std::size_t sws)
    {
        m_StatsWindowSize = sws;
    }
    
    void GyroscopeCalibration::setStatsSD(float ssd)
    {
        m_StatsSD = ssd;
    }
    
    void GyroscopeCalibration::processSample(Vec3f sample)
    {
        m_SamplesQuality = 0.0f;
        
        m_StatsX(sample.x());
        m_StatsY(sample.y());
        m_StatsZ(sample.z());
        
        if(accumulators::rolling_count(m_StatsX) < m_StatsWindowSize)
            return;
        
        m_MeanSample = Vec3f(
            accumulators::rolling_mean(m_StatsX),
            accumulators::rolling_mean(m_StatsY),
            accumulators::rolling_mean(m_StatsZ)
        );
        
        updateSamplesQuality();
    }

    
    Vec3f GyroscopeCalibration::meanSample() const
    {
        return m_MeanSample;
    }
    
    void GyroscopeCalibration::reset()
    {
        m_SamplesQuality = 0.0f;
        m_StatsX = stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsY = stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsZ = stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_MeanSample = Vec3f::Zero();
        m_MapTransform.setIdentity();
        m_CalibrationTransform.setIdentity();
    }
    
    float GyroscopeCalibration::samplesQuality() const
    {
        return m_SamplesQuality;
    }
    
    Transform3af const& GyroscopeCalibration::mapTransform() const
    {
        return m_MapTransform;
    }
    
    Transform3af const& GyroscopeCalibration::calibrationTransform() const
    {
        return m_CalibrationTransform;
    }
    
    void GyroscopeCalibration::update()
    {
        m_MapTransform.setIdentity();
        m_MapTransform.pretranslate(m_MeanSample);
        
        m_CalibrationTransform.setIdentity();
        m_CalibrationTransform.translate(-m_MeanSample);
    }
    
    template<class Archive>
    void GyroscopeCalibration::serialize(Archive &ar, const unsigned int)
    {
        ar & m_MapTransform;
        ar & m_CalibrationTransform;
    }
    
    void GyroscopeCalibration::updateSamplesQuality()
    {
        float const statsXSD = std::sqrt(accumulators::rolling_variance(m_StatsX));
        float const statsYSD = std::sqrt(accumulators::rolling_variance(m_StatsY));
        float const statsZSD = std::sqrt(accumulators::rolling_variance(m_StatsZ));
                
        if(statsXSD > m_StatsSD || statsYSD > m_StatsSD || statsZSD > m_StatsSD)
            return;
        
        m_SamplesQuality = 1.0f;
    }
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::control::GyroscopeCalibration)
