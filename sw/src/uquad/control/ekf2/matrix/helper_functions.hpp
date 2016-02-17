#ifndef UQUAD_CONTROL_EKF2_MATRIX_HELPER_FUNCTIONS_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_HELPER_FUNCTIONS_HPP

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
Type wrap_pi(Type x)
{
    if (!std::isfinite(x)) {
        return x;
    }

    while (x >= (Type)M_PI) {
        x -= (Type)(2.0 * M_PI);

    }

    while (x < (Type)(-M_PI)) {
        x += (Type)(2.0 * M_PI);

    }

    return x;
}

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_HELPER_FUNCTIONS_HPP
