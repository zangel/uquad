#ifndef UQUAD_CONTROL_EKF2_MATRIX_SCALAR_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_SCALAR_HPP

#include "../../Config.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
namespace matrix
{

template<typename Type>
class Scalar
{
public:
    Scalar() : _value()
    {
    }

    Scalar(const Matrix<Type, 1, 1> & other)
    {
        _value = other(0,0);
    }

    Scalar(float other)
    {
        _value = other;
    }

    operator Type()
    {
        return _value;
    }
private:
    Type _value;
};

typedef Scalar<float> Scalarf;

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_SCALAR_HPP
