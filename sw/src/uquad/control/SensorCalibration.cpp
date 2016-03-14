#include "SensorCalibration.h"
#include <cmath>

namespace uquad
{
namespace control
{
    namespace
    {
        static uint64_t const points_level0_shift = 0;
        static std::bitset<64> const points_level0_mask(((1 << SensorCalibration::POINTS_COUNT_L0) - 1) << points_level0_shift);
        
        static uint64_t const points_level1_shift = SensorCalibration::POINTS_COUNT_L0;
        static std::bitset<64> const points_level1_mask(((1 << SensorCalibration::POINTS_COUNT_L1) - 1) << points_level1_shift);
        
        static uint64_t const points_level2_shift = SensorCalibration::POINTS_COUNT_L0 + SensorCalibration::POINTS_COUNT_L1;
        static std::bitset<64> const points_level2_mask(((1 << SensorCalibration::POINTS_COUNT_L2) - 1) << points_level2_shift);
    }
    
    std::size_t SensorCalibration::indexOfDirection(Vector3 dir)
    {
        int iX = lround(dir.x()*10);
        int iY = lround(dir.y()*10);
        int iZ = lround(dir.z()*10);
        
        int iSX = copysign(1.0f, dir.x());
        int iSY = copysign(1.0f, dir.y());
        int iSZ = copysign(1.0f, dir.z());
        
        std::size_t index = POINTS_COUNT;
        
        //axes
        if(std::abs(iX) == 10 && !iY && !iZ)
        {
            index = 1 - ((iSX + 1) >> 1);
        }
        else
        if(std::abs(iY) == 10 && !iX && !iZ)
        {
            index = 3 - ((iSY + 1) >> 1);
        }
        else
        if(std::abs(iZ) == 10 && !iX && !iY)
        {
            index = 5 - ((iSZ + 1) >> 1);
        }
        else
        if(!iZ && std::abs(iX) == 7 && std::abs(iY) == 7)
        {
            index = POINTS_COUNT_L0 + POINTS_COUNT_L1 + 3 - ((iSY + 1) + ((iSX + 1) >> 1));
        }
        else
        if(!iY && std::abs(iX) == 7 && std::abs(iZ) == 7)
        {
            index = POINTS_COUNT_L0 + POINTS_COUNT_L1 + 7 - ((iSZ + 1) + ((iSX + 1) >> 1));
        }
        else
        if(!iX && std::abs(iY) == 7 && std::abs(iZ) == 7)
        {
            index = POINTS_COUNT_L0 + POINTS_COUNT_L1 + 11 - ((iSZ + 1) + ((iSY + 1) >> 1));
        }
        else
        if(std::abs(iX) == 6 && std::abs(iY) == 6 && std::abs(iZ) == 6)
        {
            index = POINTS_COUNT_L0 + POINTS_COUNT_L1 - 1 - (((iSZ + 1) << 1) + (iSY + 1) + ((iSX + 1) >> 1));
        }

        return index;
    }
    
    SensorCalibration::Vector3 SensorCalibration::directionOfIndex(std::size_t index)
    {
        if(index < POINTS_COUNT_L0)
        {
            float const sign = -int((index & 1) << 1) + 1;
            switch((index & 6) >> 1)
            {
            case 0: return Vector3(sign, 0.0f, 0.0f); break;
                
            case 1: return Vector3(0.0f, sign, 0.0f); break;
                
            case 2: return Vector3(0.0f, 0.0f, sign); break;
            }
        }
        
        if(index < (POINTS_COUNT_L0 + POINTS_COUNT_L1))
        {
            std::size_t const offset = index - POINTS_COUNT_L0;
            
            float const signX = -int((offset & 1) << 1) + 1;
            float const signY = -int((offset & 2) << 0) + 1;
            float const signZ = -int((offset & 4) >> 1) + 1;
            
            return Vector3(signX, signY, signZ).normalized();
        }
        
        {
            std::size_t const offset = index - POINTS_COUNT_L0 - POINTS_COUNT_L1;
            
            float const sign0 = -int((offset & 1) << 1) + 1;
            float const sign1 = -int((offset & 2) << 0) + 1;
            
            switch((offset & 12) >> 2)
            {
            case 0: return Vector3(sign0, sign1,  0.0f).normalized(); break;
            case 1: return Vector3(sign0,  0.0f, sign1).normalized(); break;
            case 2: return Vector3( 0.0f, sign0, sign1).normalized(); break;
            }
        }
        
        return Vector3::Zero();
    }
    
    
    SensorCalibration::SensorCalibration()
        : m_PointsFlags(0)
        , m_Points()
        , m_PointsQuality(0.0f)
        , m_bEllipsoidValid(false)
        , m_Ellipsoid()
        , m_StatsWindowSize(5)
        , m_StatsSD(5.0e-1f)
        , m_StatsX(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsY(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsZ(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_MeanPoint(Vector3::Zero())
    {
    }
    
    SensorCalibration::~SensorCalibration()
    {
    }
    
    void SensorCalibration::reset()
    {
        m_PointsFlags.reset();
        m_PointsQuality = 0.0f;
        m_bEllipsoidValid = false;
        m_Ellipsoid = math::geom::Ellipsoidf();
        m_StatsX = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsY = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsZ = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_MeanPoint = Vector3::Zero();
    }
    
    void SensorCalibration::process(Vector3 point)
    {
        m_StatsX(point.x());
        m_StatsY(point.y());
        m_StatsZ(point.z());
        
        if(accumulators::rolling_count(m_StatsX) < m_StatsWindowSize)
            return;
        
        float const statsXSD = std::sqrt(accumulators::rolling_variance(m_StatsX));
        float const statsYSD = std::sqrt(accumulators::rolling_variance(m_StatsY));
        float const statsZSD = std::sqrt(accumulators::rolling_variance(m_StatsZ));
                
        if(statsXSD > 5.0e-1f || statsYSD > 5.0e-1f || statsZSD > 5.0e-1f)
            return;
        
        m_MeanPoint = Vector3(
            accumulators::rolling_mean(m_StatsX),
            accumulators::rolling_mean(m_StatsY),
            accumulators::rolling_mean(m_StatsZ)
        );
            
        //reset stats
        m_StatsX = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsY = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsZ = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        
        
        Vector3 dir(m_MeanPoint);
        dir.normalize();
        
        std::size_t index = indexOfDirection(dir);
        
        
        if(index >= POINTS_COUNT)
            return;

        m_PointsFlags[index] = true;
        m_Points[index] = dir;
        
        updatePointsQuality();
        m_bEllipsoidValid = false;
    }
    
    math::geom::Ellipsoidf const& SensorCalibration::ellipsoid() const
    {
        if(!m_bEllipsoidValid && m_PointsQuality >= 0.5f)
        {
            std::vector<math::geom::Ellipsoidf::Vector3> fitPoints(m_PointsFlags.count());
            std::size_t index = 0;
            for(std::size_t p = 0; p < POINTS_COUNT; ++p)
            {
                if(!m_PointsFlags[p]) continue;
                fitPoints[index++] = m_Points[p];
            }
            
            m_Ellipsoid = math::geom::Ellipsoidf::leastSquaresFit(fitPoints);
            m_bEllipsoidValid = true;
        }
        return m_Ellipsoid;
    }
    
    void SensorCalibration::updatePointsQuality()
    {
        m_PointsQuality = 0.0f;
        if((m_PointsFlags & points_level0_mask) == points_level0_mask)
            m_PointsQuality += 0.5f;
        
        m_PointsQuality += 0.3f * static_cast<float>((m_PointsFlags & points_level1_mask).count())/POINTS_COUNT_L1;
        m_PointsQuality += 0.2f * static_cast<float>((m_PointsFlags & points_level2_mask).count())/POINTS_COUNT_L2;
    }
    
} //namespace control
} //namespace uquad
