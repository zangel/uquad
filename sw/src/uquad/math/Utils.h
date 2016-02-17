#ifndef UQUAD_MATH_UTILS_H
#define UQUAD_MATH_UTILS_H

#include "Config.h"

namespace uquad
{
namespace math
{
    
    template <typename T> static inline T square(T t) { return t*t; }
    
    template <typename T>
    static inline T radians(T t)
    {
        static T const deg2rad = static_cast<T>(0.017453292519943295769236907684886);
        return deg2rad*t;
    }
    
    template <typename T>
    static inline T wrapPi(T x)
    {
        if(!std::isfinite(x))
            return x;
        
        while(x >= T(M_PI)) { x -= T(2.0 * M_PI); }
        while(x < T(-M_PI)) { x += T(2.0 * M_PI); }
        
        return x;
    }

    
    template <typename T>
    static inline Quaternion<T> quatFromAxisAngle(Matrix<T, 3, 1> axis, T theta)
    {
        T magnitude = std::sin(theta*T(0.5));
        
        return Quaternion<T>(
            std::cos(theta*T(0.5)),
            axis(0) * magnitude,
            axis(1) * magnitude,
            axis(2) * magnitude
        );
    }
    
    template <typename T>
    static inline Quaternion<T> quatFromAxisAngle(Matrix<T, 3, 1> vec)
    {
        T theta = vec.norm();
        if(theta < T(1.0e-10))
            return Quaternion<T>::Identity();
        vec /= theta;
        return quatFromAxisAngle(vec, theta);
    }
    
    template <typename T>
    static inline Matrix<T, 3, 1> quatToAxisAngle(Quaternion<T> q)
    {
        Matrix<T, 3, 1> vec = q.vec();
        T axis_magnitude = vec.norm();
        
        if(axis_magnitude >= T(1e-10))
        {
            vec /= axis_magnitude;
            vec *= wrapPi(T(2.0 * std::atan2(axis_magnitude, q.w())));
        }
        return vec;
    }
    
    template <typename T>
    static inline T constrain(T v, T lower, T upper)
    {
        return std::max<T>(lower, std::min<T>(upper, v));
    }
    
} //namespace math
} //namespace uquad

#endif //UQUAD_MATH_UTILS_H