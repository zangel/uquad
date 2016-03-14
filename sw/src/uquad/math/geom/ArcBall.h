#ifndef UQUAD_MATH_GEOM_ARCBALL_H
#define UQUAD_MATH_GEOM_ARCBALL_H

#include "../Config.h"

namespace uquad
{
namespace math
{
namespace geom
{
    class ArcBall
    {
    public:
        ArcBall();
        ~ArcBall();
        
        void setBounds(std::size_t w, std::size_t h);
        
        void start(Vec2f p);
        Quaternionf move(Vec2f p);

    private:
        Vec3f mapToSphere(Vec2f p) const;
        
    private:
        float m_AdjustWidth;
        float m_AdjustHeight;
        Vec3f m_StartVector;
        Vec3f m_EndVector;
    };
    
} //namespace geom
} //namespace math
} //namespace uquad

#endif //UQUAD_MATH_GEOM_ARCBALL_H
