#ifndef UQUAD_CONTROL_EKF2_MATRIX_MATRIX_HPP
#define UQUAD_CONTROL_EKF2_MATRIX_MATRIX_HPP

#include "../../Config.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
namespace matrix
{

template <typename Type, size_t M>
class Vector;

template<typename Type, size_t  M, size_t N>
class Matrix
{

public:

    Type _data[M][N];

    Matrix() :
        _data()
    {
    }

    Matrix(const Type *data_) :
        _data()
    {
        memcpy(_data, data_, sizeof(_data));
    }

    Matrix(const Matrix &other) :
        _data()
    {
        memcpy(_data, other._data, sizeof(_data));
    }

    /**
     * Accessors/ Assignment etc.
     */

    Type *data()
    {
        return _data[0];
    }

    inline Type operator()(size_t i, size_t j) const
    {
        return _data[i][j];
    }

    inline Type &operator()(size_t i, size_t j)
    {
        return _data[i][j];
    }

    /**
     * Matrix Operations
     */

    // this might use a lot of programming memory
    // since it instantiates a class for every
    // required mult pair, but it provides
    // compile time size_t checking
    template<size_t P>
    Matrix<Type, M, P> operator*(const Matrix<Type, N, P> &other) const
    {
        const Matrix<Type, M, N> &self = *this;
        Matrix<Type, M, P> res;
        res.setZero();

        for (size_t i = 0; i < M; i++) {
            for (size_t k = 0; k < P; k++) {
                for (size_t j = 0; j < N; j++) {
                    res(i, k) += self(i, j) * other(j, k);
                }
            }
        }

        return res;
    }

    Matrix<Type, M, N> emult(const Matrix<Type, M, N> &other) const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = self(i, j)*other(i, j);
            }
        }

        return res;
    }

    Matrix<Type, M, N> operator+(const Matrix<Type, M, N> &other) const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = self(i, j) + other(i, j);
            }
        }

        return res;
    }

    bool operator==(const Matrix<Type, M, N> &other) const
    {
        const Matrix<Type, M, N> &self = *this;
        static const Type eps = Type(1e-4);

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                if (fabs(self(i , j) - other(i, j)) > eps) {
                    return false;
                }
            }
        }

        return true;
    }

    Matrix<Type, M, N> operator-(const Matrix<Type, M, N> &other) const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = self(i, j) - other(i, j);
            }
        }

        return res;
    }

    // unary minus
    Matrix<Type, M, N> operator-() const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = -self(i, j);
            }
        }

        return res;
    }

    void operator+=(const Matrix<Type, M, N> &other)
    {
        Matrix<Type, M, N> &self = *this;
        self = self + other;
    }

    void operator-=(const Matrix<Type, M, N> &other)
    {
        Matrix<Type, M, N> &self = *this;
        self = self - other;
    }

    template<size_t P>
    void operator*=(const Matrix<Type, N, P> &other)
    {
        Matrix<Type, M, N> &self = *this;
        self = self * other;
    }

    /**
     * Scalar Operations
     */

    Matrix<Type, M, N> operator*(Type scalar) const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = self(i, j) * scalar;
            }
        }

        return res;
    }

    inline Matrix<Type, M, N> operator/(Type scalar) const
    {
        return (*this)*(1/scalar);
    }

    Matrix<Type, M, N> operator+(Type scalar) const
    {
        Matrix<Type, M, N> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(i , j) = self(i, j) + scalar;
            }
        }

        return res;
    }

    inline Matrix<Type, M, N> operator-(Type scalar) const
    {
        return (*this) + (-1*scalar);
    }

    void operator*=(Type scalar)
    {
        Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                self(i, j) = self(i, j) * scalar;
            }
        }
    }

    void operator/=(Type scalar)
    {
        Matrix<Type, M, N> &self = *this;
        self = self * (1.0f / scalar);
    }

    inline void operator+=(Type scalar)
    {
        *this = (*this) + scalar;
    }

    inline void operator-=(Type scalar)
    {
        *this = (*this) - scalar;
    }


    Matrix<Type, N, M> transpose() const
    {
        Matrix<Type, N, M> res;
        const Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                res(j, i) = self(i, j);
            }
        }

        return res;
    }

    // tranpose alias
    inline Matrix<Type, N, M> T() const
    {
        return transpose();
    }

    void setZero()
    {
        memset(_data, 0, sizeof(_data));
    }

    void setAll(Type val)
    {
        Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            for (size_t j = 0; j < N; j++) {
                self(i, j) = val;
            }
        }
    }

    inline void setOne()
    {
        setAll(1);
    }

    void setIdentity()
    {
        setZero();
        Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M and i < N; i++) {
            self(i, i) = 1;
        }
    }

    inline void swapRows(size_t a, size_t b)
    {
        if (a == b) {
            return;
        }

        Matrix<Type, M, N> &self = *this;

        for (size_t j = 0; j < N; j++) {
            Type tmp = self(a, j);
            self(a, j) = self(b, j);
            self(b, j) = tmp;
        }
    }

    inline void swapCols(size_t a, size_t b)
    {
        if (a == b) {
            return;
        }

        Matrix<Type, M, N> &self = *this;

        for (size_t i = 0; i < M; i++) {
            Type tmp = self(i, a);
            self(i, a) = self(i, b);
            self(i, b) = tmp;
        }
    }

    Matrix<Type, M, N> abs()
    {
        Matrix<Type, M, N> r;
        for (size_t i=0; i<M; i++) {
            for (size_t j=0; j<M; j++) {
                r(i,j) = Type(fabs((*this)(i,j)));
            }
        }
        return r;
    }

    Type max()
    {
        Type max_val = (*this)(0,0);
        for (size_t i=0; i<M; i++) {
            for (size_t j=0; j<M; j++) {
                Type val = (*this)(i,j);
                if (val > max_val) {
                    max_val = val;
                }
            }
        }
        return max_val;
    }

    Type min()
    {
        Type min_val = (*this)(0,0);
        for (size_t i=0; i<M; i++) {
            for (size_t j=0; j<M; j++) {
                Type val = (*this)(i,j);
                if (val < min_val) {
                    min_val = val;
                }
            }
        }
        return min_val;
    }

};

template<typename Type, size_t M, size_t N>
Matrix<Type, M, N> zeros() {
    Matrix<Type, M, N> m;
    m.setZero();
    return m;
}

template<typename Type, size_t M, size_t N>
Matrix<Type, M, N> ones() {
    Matrix<Type, M, N> m;
    m.setOne();
    return m;
}

template<typename Type, size_t  M, size_t N>
Matrix<Type, M, N> operator*(Type scalar, const Matrix<Type, M, N> &other)
{
    return other * scalar;
}

template<typename Type, size_t  M, size_t N>
bool isEqual(const Matrix<Type, M, N> &x,
             const Matrix<Type, M, N> & y) {
    if (!(x == y)) {
        char buf_x[100];
        char buf_y[100];
        x.write_string(buf_x, 100);
        y.write_string(buf_y, 100);
    }
    return x == y;
}

typedef Matrix<float, 3, 3> Matrix3f;

} //namespace matrix
} //namespace ekf2
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_EKF2_MATRIX_MATRIX_HPP
