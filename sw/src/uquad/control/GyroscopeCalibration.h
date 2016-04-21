#ifndef UQUAD_CONTROL_GYROSCOPE_CALIBRATION_H
#define UQUAD_CONTROL_GYROSCOPE_CALIBRATION_H

#include "Config.h"
#include "../base/Serialization.h"

namespace uquad
{
namespace control
{
    class GyroscopeCalibration
    {
    public:
        typedef accumulators::accumulator_set
        <
            float,
            accumulators::stats
            <
                accumulators::tag::rolling_mean,
                accumulators::tag::rolling_variance
            >
        > stats_t;
        
        GyroscopeCalibration();
        ~GyroscopeCalibration();
        
        void setStatsWindowSize(std::size_t sws);
        void setStatsSD(float ssd);
        
        void processSample(Vec3f sample);

        Vec3f meanSample() const;
        
        void reset();
        void update();
        
        float samplesQuality() const;
        
        Transform3af const& mapTransform() const;
        Transform3af const& calibrationTransform() const;
        
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
        
    private:
        void updateSamplesQuality();
        
    protected:
        std::size_t m_StatsWindowSize;
        float m_StatsSD;
        
        stats_t m_StatsX;
        stats_t m_StatsY;
        stats_t m_StatsZ;
        
        Vec3f m_MeanSample;
        
        float m_SamplesQuality;
        
        Transform3af m_MapTransform;
        Transform3af m_CalibrationTransform;
    };
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::control::GyroscopeCalibration)

#endif //UQUAD_CONTROL_GYROSCOPE_CALIBRATION_H
