#include "ArcBall.h"

namespace uquad
{
namespace math
{
namespace geom
{
    ArcBall::ArcBall()
        : m_AdjustWidth(0.0f)
        , m_AdjustHeight(0.0f)
        , m_StartVector()
        , m_EndVector()
    
    {
    }
    
    ArcBall::~ArcBall()
    {
    }
    
    void ArcBall::setBounds(std::size_t w, std::size_t h)
    {
        m_AdjustWidth = 1.0f/((w - 1.0f) * 0.5f);
        m_AdjustHeight = 1.0f/((h - 1.0f) * 0.5f);
        
    }
    
    void ArcBall::start(Vec2f p)
    {
        m_StartVector = mapToSphere(p);
    }
    
    Quaternionf ArcBall::move(Vec2f p)
    {
        m_EndVector = mapToSphere(p);
        
        Vec3f perp = m_StartVector.cross(m_EndVector);
        if(perp.norm() > 1.0e-5f)
        {
            return Quaternionf(
                m_StartVector.dot(m_EndVector),
                perp.x(),
                perp.y(),
                perp.z()
            );
        }
        
        return Quaternionf::Identity();
    }
    
    Vec3f ArcBall::mapToSphere(Vec2f p) const
    {
        Vec2f temp = Vec2f(
            p.x() * m_AdjustWidth -1.0f,
            1.0f - p.y() * m_AdjustHeight
        );
        
        float length2 = temp.x() * temp.x() + temp.y() * temp.y();
        
        if(length2 > 1.0f)
        {
            
            float norm = 1.0f/std::sqrt(length2);
            return Vec3f(
                temp.x()*norm,
                temp.y()*norm,
                0.0f
            );
        }
        
        return Vec3f(
            temp.x(),
            temp.y(),
            std::sqrt(1.0f - length2)
        );
    }
    
    
} //namespace geom
} //namespace math
} //namespace uquad