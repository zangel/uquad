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
        typedef accumulators::accumulator_set
        <
            float,
            accumulators::stats
            <
                accumulators::tag::rolling_mean,
                accumulators::tag::rolling_variance
            >
        > real_stats_t;
        
        AccelerometerCalibration();
        ~AccelerometerCalibration();
        
        
        static std::size_t const POINTS_COUNT_L0 = 6;
        static std::size_t const POINTS_COUNT_L1 = 8;
        static std::size_t const POINTS_COUNT_L2 = 12;
        static std::size_t const POINTS_COUNT = POINTS_COUNT_L0 + POINTS_COUNT_L1 + POINTS_COUNT_L2;
        
        static std::size_t indexOfDirection(Vec3f dir);
        static Vec3f directionOfIndex(std::size_t index);
        
        
        inline Transform3af const& mapTransform() const { return m_MapTransform; }
        inline Transform3af const& calibrationTransform() const { return m_CalibrationTransform; }
        
        
        void setStatsWindowSize(std::size_t sws);
        void setStatsSD(float ssd);
        
        void reset();
        void process(Vec3f point);
        
        inline float pointsQuality() const { return m_PointsQuality; }
        inline Vec3f meanPoint() const { return m_MeanPoint; }
        
        inline bool isDirectionSampled(std::size_t index) const { return m_PointsFlags.test(index); }
        
        void update();
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
        
    private:
        void updatePointsQuality();
        
    protected:
        std::bitset<64> m_PointsFlags;
        Vec3f m_Points[POINTS_COUNT];
        float m_PointsQuality;
        std::size_t m_StatsWindowSize;
        float m_StatsSD;
        real_stats_t m_StatsX;
        real_stats_t m_StatsY;
        real_stats_t m_StatsZ;
        Vec3f m_MeanPoint;
        Transform3af m_MapTransform;
        Transform3af m_CalibrationTransform;
    };
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::control::AccelerometerCalibration)

#endif //UQUAD_CONTROL_ACCELEROMETER_CALIBRATION_H
