#include "AccelerometerCalibration.h"
#include "../math/geom/Ellipsoid.h"
#include "../utility/EigenMatrixSerialization.h"

#include <cmath>

namespace uquad
{
namespace control
{
    namespace
    {
        static uint64_t const points_level0_shift = 0;
        static std::bitset<64> const points_level0_mask(((1 << AccelerometerCalibration::POINTS_COUNT_L0) - 1) << points_level0_shift);
        
        static uint64_t const points_level1_shift = AccelerometerCalibration::POINTS_COUNT_L0;
        static std::bitset<64> const points_level1_mask(((1 << AccelerometerCalibration::POINTS_COUNT_L1) - 1) << points_level1_shift);
        
        static uint64_t const points_level2_shift = AccelerometerCalibration::POINTS_COUNT_L0 + AccelerometerCalibration::POINTS_COUNT_L1;
        static std::bitset<64> const points_level2_mask(((1 << AccelerometerCalibration::POINTS_COUNT_L2) - 1) << points_level2_shift);
    }
    
    std::size_t AccelerometerCalibration::indexOfDirection(Vec3f dir)
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
    
    Vec3f AccelerometerCalibration::directionOfIndex(std::size_t index)
    {
        if(index < POINTS_COUNT_L0)
        {
            float const sign = -int((index & 1) << 1) + 1;
            switch((index & 6) >> 1)
            {
            case 0: return Vec3f(sign, 0.0f, 0.0f); break;
                
            case 1: return Vec3f(0.0f, sign, 0.0f); break;
                
            case 2: return Vec3f(0.0f, 0.0f, sign); break;
            }
        }
        
        if(index < (POINTS_COUNT_L0 + POINTS_COUNT_L1))
        {
            std::size_t const offset = index - POINTS_COUNT_L0;
            
            float const signX = -int((offset & 1) << 1) + 1;
            float const signY = -int((offset & 2) << 0) + 1;
            float const signZ = -int((offset & 4) >> 1) + 1;
            
            return Vec3f(signX, signY, signZ).normalized();
        }
        
        {
            std::size_t const offset = index - POINTS_COUNT_L0 - POINTS_COUNT_L1;
            
            float const sign0 = -int((offset & 1) << 1) + 1;
            float const sign1 = -int((offset & 2) << 0) + 1;
            
            switch((offset & 12) >> 2)
            {
            case 0: return Vec3f(sign0, sign1,  0.0f).normalized(); break;
            case 1: return Vec3f(sign0,  0.0f, sign1).normalized(); break;
            case 2: return Vec3f( 0.0f, sign0, sign1).normalized(); break;
            }
        }
        
        return Vec3f::Zero();
    }
    
    
    AccelerometerCalibration::AccelerometerCalibration()
        : m_PointsFlags(0)
        , m_Points()
        , m_PointsQuality(0.0f)
        , m_StatsWindowSize(5)
        , m_StatsSD(5.0e-3f)
        , m_StatsX(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsY(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_StatsZ(accumulators::tag::rolling_window::window_size = m_StatsWindowSize)
        , m_MeanPoint(Vec3f::Zero())
        , m_MapTransform(Transform3af::Identity())
        , m_CalibrationTransform(Transform3af::Identity())
    {
    }
    
    AccelerometerCalibration::~AccelerometerCalibration()
    {
    }
    
    void AccelerometerCalibration::setStatsWindowSize(std::size_t sws)
    {
        m_StatsWindowSize = sws;
    }
    
    void AccelerometerCalibration::setStatsSD(float ssd)
    {
        m_StatsSD = ssd;
    }
    
    void AccelerometerCalibration::reset()
    {
        m_PointsFlags.reset();
        m_PointsQuality = 0.0f;
        m_StatsX = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsY = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsZ = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_MeanPoint = Vec3f::Zero();
        m_MapTransform.setIdentity();
        m_CalibrationTransform.setIdentity();
    }
    
    void AccelerometerCalibration::process(Vec3f point)
    {
        m_StatsX(point.x());
        m_StatsY(point.y());
        m_StatsZ(point.z());
        
        if(accumulators::rolling_count(m_StatsX) < m_StatsWindowSize)
            return;
        
        m_MeanPoint = Vec3f(
            accumulators::rolling_mean(m_StatsX),
            accumulators::rolling_mean(m_StatsY),
            accumulators::rolling_mean(m_StatsZ)
        );
        
        float const statsXSD = std::sqrt(accumulators::rolling_variance(m_StatsX));
        float const statsYSD = std::sqrt(accumulators::rolling_variance(m_StatsY));
        float const statsZSD = std::sqrt(accumulators::rolling_variance(m_StatsZ));
                
        if(statsXSD > m_StatsSD || statsYSD > m_StatsSD || statsZSD > m_StatsSD)
            return;
        
        //reset stats
        m_StatsX = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsY = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        m_StatsZ = real_stats_t(accumulators::tag::rolling_window::window_size = m_StatsWindowSize);
        
        Vec3f dir(m_MeanPoint);
        dir.normalize();
        
        std::size_t index = indexOfDirection(dir);
        
        
        if(index >= POINTS_COUNT)
            return;

        m_PointsFlags[index] = true;
        m_Points[index] = m_MeanPoint;
        
        updatePointsQuality();
    }
    
    void AccelerometerCalibration::update()
    {
        if(m_PointsQuality < 0.5f)
            return;
        
        #if 0
        #if 0
        std::size_t nPoints = POINTS_COUNT_L0;//m_PointsFlags.count();
        Vec3f center = Vec3f::Zero();
        for(std::size_t p = 0; p < POINTS_COUNT_L0; ++p)
        {
            center += m_Points[p];
        }
        
        center /= nPoints;
        #else
        Vec3f center = Vec3f(
            m_Points[0].x() + m_Points[1].x(),
            m_Points[2].y() + m_Points[3].y(),
            m_Points[4].z() + m_Points[5].z()
        );
        center *= 0.5f;
        #endif
        
        Vec3f radii = Vec3f(
            (m_Points[0] - center).norm() + (m_Points[1] - center).norm(),
            (m_Points[2] - center).norm() + (m_Points[3] - center).norm(),
            (m_Points[4] - center).norm() + (m_Points[5] - center).norm()
        );
        radii *= 0.5f;
        
        m_MapTransform.setIdentity();
        m_MapTransform.scale(radii);
        m_MapTransform.pretranslate(center);
        
        m_CalibrationTransform.setIdentity();
        m_CalibrationTransform.prescale(Vec3f(1/radii.x(), 1/radii.y(), 1/radii.z()));
        m_CalibrationTransform.translate(-center);
        #else
        std::vector< math::geom::Ellipsoidf::Vector3 > points;
        for(std::size_t p = 0; p < POINTS_COUNT; ++p)
        {
            if(!m_PointsFlags[p]) continue;
            points.push_back(m_Points[p]);
        }
        math::geom::Ellipsoidf ellipsoid = math::geom::Ellipsoidf::leastSquaresFit(points);
        m_MapTransform = ellipsoid.mapTransform();
        m_CalibrationTransform = ellipsoid.unmapTransform();
        m_CalibrationTransform.rotate(ellipsoid.eigenVectors());
        #endif
    }
    
    template<class Archive>
    void AccelerometerCalibration::serialize(Archive &ar, const unsigned int)
    {
        ar & m_MapTransform;
        ar & m_CalibrationTransform;
    }
    
    void AccelerometerCalibration::updatePointsQuality()
    {
        m_PointsQuality = 0.0f;
        if((m_PointsFlags & points_level0_mask) == points_level0_mask)
            m_PointsQuality += 0.5f;
        
        m_PointsQuality += 0.3f * static_cast<float>((m_PointsFlags & points_level1_mask).count())/POINTS_COUNT_L1;
        m_PointsQuality += 0.2f * static_cast<float>((m_PointsFlags & points_level2_mask).count())/POINTS_COUNT_L2;
    }
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::control::AccelerometerCalibration)