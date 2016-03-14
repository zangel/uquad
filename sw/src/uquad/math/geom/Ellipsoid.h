#ifndef UQUAD_MATH_GEOM_ELLIPSOID_H
#define UQUAD_MATH_GEOM_ELLIPSOID_H

#include "../Config.h"

namespace uquad
{
namespace math
{
namespace geom
{
    template <typename Scalar>
    class Ellipsoid
    {
    public:
        typedef Eigen::Matrix<Scalar, 3, 1> Vector3;
        typedef Eigen::Matrix<Scalar, 3, 3> Matrix3x3;
        typedef Eigen::Matrix<Scalar, 4, 4> Matrix4x4;
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> MatrixX;
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> VectorX;
        typedef Eigen::Transform<Scalar, 3, Eigen::Affine> Affine;
        
        
        
        Ellipsoid()
            : m_Center(Vector3::Zero())
            , m_Radii(Vector3::Zero())
            , m_EV(Matrix3x3::Zero())
            , m_EVT(Matrix3x3::Zero())
            , m_Map()
            , m_MapInv()
        {
        }
        
        
        Ellipsoid(Vector3 center, Vector3 radii, Matrix3x3 const &ev)
            : m_Center(center)
            , m_Radii(radii)
            , m_EV(ev)
            , m_EVT(ev.transpose())
            , m_Map()
            , m_MapInv()
        {
            updateMatrix();
        }
        
        Ellipsoid& operator =(Ellipsoid const &rhs)
        {
            m_Center = rhs.m_Center;
            m_Radii = rhs.m_Radii;
            m_EV = rhs.m_EV;
            m_EVT = rhs.m_EVT;
            m_Map = rhs.m_Map;
            m_MapInv = rhs.m_MapInv;
            return *this;
        }
        
        static Ellipsoid inertiaTensorFit(std::vector<Vector3> const &points)
        {
            Vector3 center = Vector3::Zero();
            for(std::size_t p = 0; p < points.size(); ++p)
            {
                center += points[p];
            }
            
            center /= points.size();
            
            Scalar Icxx = 0;
            Scalar Icyy = 0;
            Scalar Iczz = 0;
            Scalar Icxy = 0;
            Scalar Icxz = 0;
            Scalar Icyz = 0;
            
            for(std::size_t p = 0; p < points.size(); ++p)
            {
                Vector3 t = points[p] - center;
                Scalar xx = t.x() * t.x();
                Scalar yy = t.y() * t.y();
                Scalar zz = t.z() * t.z();
                
                Icxx += yy + zz;
                Icyy += xx + zz;
                Iczz += xx + yy;
                Icxy += t.x() * t.y();
                Icxz += t.x() * t.z();
                Icyz += t.y() * t.z();
            }
            
            Icxx /= points.size();
            Icyy /= points.size();
            Iczz /= points.size();
            Icxy /= points.size();
            Icxz /= points.size();
            Icyz /= points.size();
            
            
            Matrix3x3 em;
            em <<
                 Icxx, -Icxy, -Icxz,
                -Icxy,  Icyy, -Icyz,
                -Icxz, -Icyz,  Iczz;
            
            Eigen::EigenSolver<Matrix3x3> eigenSolver(em);
            
            Vector3 eigenValues = eigenSolver.pseudoEigenvalueMatrix().diagonal();
            Matrix3x3 eigenVectors = eigenSolver.pseudoEigenvectors();
            
            return Ellipsoid(
                center,
                Vector3(
                    1 / std::sqrt(eigenValues.z()),
                    1 / std::sqrt(eigenValues.y()),
                    1 / std::sqrt(eigenValues.x())
                ),
                eigenVectors
            );
        }
        
        static Ellipsoid leastSquaresFit(std::vector<Vector3> const &points)
        {
            MatrixX D(points.size(), 9);
            for(std::size_t p = 0; p < points.size(); ++p)
            {
                Vector3 const &pt = points[p];
                D(p, 0) = pt.x() * pt.x();
                D(p, 1) = pt.y() * pt.y();
                D(p, 2) = pt.z() * pt.z();
                D(p, 3) = 2 * pt.x() * pt.y();
                D(p, 4) = 2 * pt.x() * pt.z();
                D(p, 5) = 2 * pt.y() * pt.z();
                D(p, 6) = 2 * pt.x();
                D(p, 7) = 2 * pt.y();
                D(p, 8) = 2 * pt.z();
            }
            
            static MatrixX const ones = MatrixX::Ones(points.size(), 1);
            MatrixX const DT = D.transpose();
            MatrixX V = (DT*D).inverse()*DT*ones;
            
            Vector3 center, eigenValues;
            Matrix3x3 eigenVectors;
            
            leastSquaresSolve(V, center, eigenValues, eigenVectors);
            
            Vector3 radii(
                1/std::sqrt(eigenValues(0)),
                1/std::sqrt(eigenValues(1)),
                1/std::sqrt(eigenValues(2))
            );
            
            return Ellipsoid(center, radii, eigenVectors);
        }
        
        static void leastSquaresSolve(MatrixX const &eq, Vector3 &center, Vector3 &eigenValues, Matrix3x3 &eigenVectors)
        {
            Matrix4x4 A;
            A <<
                eq(0, 0), eq(3, 0), eq(4, 0), eq(6, 0),
                eq(3, 0), eq(1, 0), eq(5, 0), eq(7, 0),
                eq(4, 0), eq(5, 0), eq(2, 0), eq(8, 0),
                eq(6, 0), eq(7, 0), eq(8, 0), -1;
            
            center = (-1 * A.template block<3, 3>(0, 0)).inverse() * eq.template block<3, 1>(6, 0);
            Matrix4x4 T = Matrix4x4::Identity();
            T.template block<1, 3>(3, 0) = center;
            
            Matrix4x4 R = T * A * T.transpose();
            Scalar r33 = R(3,3);
            Matrix3x3 R02 = R.template block<3, 3>(0,0);
            R02 *= -1/r33;
            
            Eigen::EigenSolver<Matrix3x3> eigenSolver(R02);
            
            eigenValues = eigenSolver.pseudoEigenvalueMatrix().diagonal();
            eigenVectors = eigenSolver.pseudoEigenvectors();
        }
        
        
        
        void map(Vector3 &vec)
        {
            vec = m_Map * vec;
        }
        
        void map(std::vector<Vector3> &pts)
        {
            std::for_each(pts.begin(), pts.end(), [this](Vector3 &pt){ map(pt); });
        }


        void unmap(Vector3 &vec)
        {
            vec = m_MapInv * vec;
        }
        
        void unmap(std::vector<Vector3> &pts)
        {
            std::for_each(pts.begin(), pts.end(), [this](Vector3 &pt){ unmap(pt); });
        }
        
        Matrix3x3 const& eigenVectors() const { return m_EV; }
        Matrix3x3 const& eigenVectorsTransposed() const { return m_EVT; }
        
        Affine const& mapTransform() const { return m_Map; }
        Affine const& unmapTransform() const { return m_MapInv; }
        
        
    private:
        void updateMatrix()
        {
            m_Map.setIdentity();
            m_Map.scale(m_Radii);
            m_Map.rotate(m_EV);
            m_Map.pretranslate(m_Center);
            
            m_MapInv.setIdentity();
            m_MapInv.prescale(Vector3(1/m_Radii.x(), 1/m_Radii.y(), 1/m_Radii.z()));
            m_MapInv.prerotate(m_EVT);
            m_MapInv.translate(-m_Center);
            
            //m_MapInv = m_Map.inverse();
        }
        
    private:
        Vector3 m_Center;
        Vector3 m_Radii;
        Matrix3x3 m_EV;
        Matrix3x3 m_EVT;
        Affine m_Map;
        Affine m_MapInv;
    };
    
    typedef Ellipsoid<float> Ellipsoidf;
    
} //namespace geom
} //namespace math
} //namespace uquad

#endif //UQUAD_MATH_GEOM_ELLIPSOID_H
