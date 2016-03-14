#ifndef UQUAD_CONTROL_SENSOR_CALIBRATION_H
#define UQUAD_CONTROL_SENSOR_CALIBRATION_H

#include "Config.h"
#include "../math/geom/Ellipsoid.h"

namespace uquad
{
namespace control
{
    class SensorCalibration
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
        
        typedef math::geom::Ellipsoidf::Vector3 Vector3;
        
        
        SensorCalibration();
        ~SensorCalibration();
        
        
        static std::size_t const POINTS_COUNT_L0 = 6;
        static std::size_t const POINTS_COUNT_L1 = 8;
        static std::size_t const POINTS_COUNT_L2 = 12;
        static std::size_t const POINTS_COUNT = POINTS_COUNT_L0 + POINTS_COUNT_L1 + POINTS_COUNT_L2;
        
        static std::size_t indexOfDirection(Vector3 dir);
        static Vector3 directionOfIndex(std::size_t index);
        
        void reset();
        void process(Vector3 point);
        
        inline float pointsQuality() const { return m_PointsQuality; }
        inline Vector3 meanPoint() const { return m_MeanPoint; }
        
        inline bool isDirectionSampled(std::size_t index) const { return m_PointsFlags.test(index); }
        
        math::geom::Ellipsoidf const& ellipsoid() const;
        
    private:
        void updatePointsQuality();
        
    protected:
        std::bitset<64> m_PointsFlags;
        Vector3 m_Points[POINTS_COUNT];
        float m_PointsQuality;
        mutable bool m_bEllipsoidValid;
        mutable math::geom::Ellipsoidf m_Ellipsoid;
        std::size_t m_StatsWindowSize;
        float m_StatsSD;
        real_stats_t m_StatsX;
        real_stats_t m_StatsY;
        real_stats_t m_StatsZ;
        Vector3 m_MeanPoint;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_SENSOR_CALIBRATION_H
