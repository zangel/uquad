#ifndef UQUAD_CONTROL_EKF2_MATRIX_DCM_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_DCM_HPP

#include "../../Config.h"
#include "Matrix.hpp"

namespace uquad
{
namespace control
{
namespace ekf2
{
namespace matrix
{

template<typename Type>
class Quaternion;

template<typename Type>
class Euler;

template<typename Type>
class Dcm : public Matrix<Type, 3, 3>
{
public:
    typedef Matrix<Type, 3, 1> Vector3;

    Dcm() : Matrix<Type, 3, 3>()
    {
        (*this) = eye<Type, 3>();
    }

    Dcm(const Type *data_) : Matrix<Type, 3, 3>(data_)
    {
    }

    Dcm(const Matrix<Type, 3, 3> &other) : Matrix<Type, 3, 3>(other)
    {
    }

    Dcm(const Quaternion<Type> & q) {
        Dcm &dcm = *this;
        Type a = q(0);
        Type b = q(1);
        Type c = q(2);
        Type d = q(3);
        Type aSq = a*a;
        Type bSq = b*b;
        Type cSq = c*c;
        Type dSq = d*d;
        dcm(0, 0) = aSq + bSq - cSq - dSq;
        dcm(0, 1) = 2 * (b * c - a * d);
        dcm(0, 2) = 2 * (a * c + b * d);
        dcm(1, 0) = 2 * (b * c + a * d);
        dcm(1, 1) = aSq - bSq + cSq - dSq;
        dcm(1, 2) = 2 * (c * d - a * b);
        dcm(2, 0) = 2 * (b * d - a * c);
        dcm(2, 1) = 2 * (a * b + c * d);
        dcm(2, 2) = aSq - bSq - cSq + dSq;
    }

    Dcm(const Euler<Type> & euler) {
        Dcm &dcm = *this;
        Type cosPhi = Type(cos(euler.phi()));
        Type sinPhi = Type(sin(euler.phi()));
        Type cosThe = Type(cos(euler.theta()));
        Type sinThe = Type(sin(euler.theta()));
        Type cosPsi = Type(cos(euler.psi()));
        Type sinPsi = Type(sin(euler.psi()));

        dcm(0, 0) = cosThe * cosPsi;
        dcm(0, 1) = -cosPhi * sinPsi + sinPhi * sinThe * cosPsi;
        dcm(0, 2) = sinPhi * sinPsi + cosPhi * sinThe * cosPsi;

        dcm(1, 0) = cosThe * sinPsi;
        dcm(1, 1) = cosPhi * cosPsi + sinPhi * sinThe * sinPsi;
        dcm(1, 2) = -sinPhi * cosPsi + cosPhi * sinThe * sinPsi;

        dcm(2, 0) = -sinThe;
        dcm(2, 1) = sinPhi * cosThe;
        dcm(2, 2) = cosPhi * cosThe;
    }
};

typedef Dcm<float> Dcmf;

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_DCM_HPP
