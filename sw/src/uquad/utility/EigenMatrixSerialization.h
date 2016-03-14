#ifndef UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H
#define UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H

#include <boost/serialization/array.hpp>

namespace boost
{
namespace serialization
{
    template <class Archive, class Scalar, int RowsAtCompileTime, int ColsAtCompileTime>
    void serialize(Archive & ar, ::Eigen::Matrix<Scalar, RowsAtCompileTime, ColsAtCompileTime> &m, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_array(m.data(), RowsAtCompileTime * ColsAtCompileTime);
    }
    
    template <class Archive, class Scalar, int Dim, int Mode, int Options>
    void serialize(Archive & ar, ::Eigen::Transform<Scalar, Dim, Mode, Options> &t, const unsigned int /*version*/)
    {
        ar & boost::serialization::make_array(
            t.matrix().data(),
            ::Eigen::Transform<Scalar, Dim, Mode, Options>::MatrixType::RowsAtCompileTime *
            ::Eigen::Transform<Scalar, Dim, Mode, Options>::MatrixType::ColsAtCompileTime
        );
    }

} //namespace serialization
} //namespace boost

#endif //UQUAD_UTILITY_EIGEN_MATRIX_SERIALIZATION_H
