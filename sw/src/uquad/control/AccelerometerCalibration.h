#ifndef UQUAD_CONTROL_ACCELEROMETER_CALIBRATION_H
#define UQUAD_CONTROL_ACCELEROMETER_CALIBRATION_H

#include "Config.h"
#include "../base/Serialization.h"

namespace uquad
{
namespace control
{
    class AccelerometerCalibration
    {
    public:
    
        static std::size_t const DIRECTION_COUNT_L0 = 6;
        static std::size_t const DIRECTION_COUNT_L1 = 8;
        static std::size_t const DIRECTION_COUNT_L2 = 12;
        static std::size_t const DIRECTION_COUNT = DIRECTION_COUNT_L0 + DIRECTION_COUNT_L1 + DIRECTION_COUNT_L2;
        
        static std::size_t directionIndex(Vec3f dir);
        static Vec3f direction(std::size_t index);
        
        typedef accumulators::accumulator_set
        <
            float,
            accumulators::stats
            <
                accumulators::tag::rolling_mean,
                accumulators::tag::rolling_variance
            >
        > stats_t;
        
        AccelerometerCalibration();
        ~AccelerometerCalibration();
        
        
        void setStatsWindowSize(std::size_t sws);
        void setStatsSD(float ssd);
        
        void processSample(Vec3f sample);
        bool isDirectionSampled(std::size_t index) const;
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
        
        std::bitset<64> m_DirectionSampled;
        Vec3f m_Samples[DIRECTION_COUNT];
        float m_SamplesQuality;
        
        Transform3af m_MapTransform;
        Transform3af m_CalibrationTransform;
    };
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::control::AccelerometerCalibration)

#endif //UQUAD_CONTROL_ACCELEROMETER_CALIBRATION_H
