#ifndef UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H
#define UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H

#include <boost/serialization/array.hpp>

namespace boost
{
namespace serialization
{
    template <class Archive, class Scalar, int RowsAtCompileTime, int ColsAtCompileTime>
    void serialize(Archive & ar, ::Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime> & m, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_array(m.data(), RowsAtCompileTime * ColsAtCompileTime);
    }

} //namespace serialization
} //namespace boost

#endif //UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H
