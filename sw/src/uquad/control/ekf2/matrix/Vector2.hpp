#ifndef UQUAD_CONTROL_EKF2_MATRIX_VECTOR2_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_VECTOR2_HPP

#include "../../Config.h"
#include "Vector.hpp"

namespace uquad
{
namespace control
{
namespace ekf2
{
namespace matrix
{

template<typename Type>
class Vector2 : public Vector<Type, 2>
{
public:

    typedef Matrix<Type, 2, 1> Matrix21;


    Vector2() :
        Vector<Type, 2>()
    {
    }

    Vector2(const Matrix21 & other) :
        Vector<Type, 2>(other)
    {
    }

    Vector2(const Type *data_) :
        Vector<Type, 2>(data_)
    {
    }

    Vector2(Type x, Type y) : Vector<Type, 2>()
    {
        Vector2 &v(*this);
        v(0) = x;
        v(1) = y;
    }

    Type cross(const Matrix21 & b)  const {
        const Vector2 &a(*this);
        return a(0)*b(1, 0) - a(1)*b(0, 0);
    }

    Type operator%(const Matrix21 & b) const {
        return (*this).cross(b);
    }

};

typedef Vector2<float> Vector2f;

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_VECTOR2_HPP
