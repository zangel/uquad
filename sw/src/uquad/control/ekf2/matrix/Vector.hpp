#ifndef UQUAD_CONTROL_EKF2_MATRIX_VECTOR_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_VECTOR_HPP

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

template<typename Type, size_t M>
class Vector : public Matrix<Type, M, 1>
{
public:
    typedef Matrix<Type, M, 1> MatrixM1;

    Vector() : MatrixM1()
    {
    }

    Vector(const MatrixM1 & other) :
        MatrixM1(other)
    {
    }

    Vector(const Type *data_) :
        MatrixM1(data_)
    {
    }

    inline Type operator()(size_t i) const
    {
        const MatrixM1 &v = *this;
        return v(i, 0);
    }

    inline Type &operator()(size_t i)
    {
        MatrixM1 &v = *this;
        return v(i, 0);
    }

    Type dot(const MatrixM1 & b) const {
        const Vector &a(*this);
        Type r = 0;
        for (size_t i = 0; i<M; i++) {
            r += a(i)*b(i,0);
        }
        return r;
    }

    Type norm() const {
        const Vector &a(*this);
        return Type(sqrt(a.dot(a)));
    }

    inline void normalize() {
        (*this) /= norm();
    }

};

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_VECTOR_HPP
