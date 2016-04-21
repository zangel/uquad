#include "MagnetometerCalibration.h"
#include "../utility/EigenMatrixSerialization.h"

#define MINMEASUREMENTS4CAL 100			// minimum number of measurements for 4 element calibration
#define MINMEASUREMENTS7CAL 150			// minimum number of measurements for 7 element calibration
#define MINMEASUREMENTS10CAL 250		// minimum number of measurements for 10 element calibration
#define MAXMEASUREMENTS 340				// maximum number of measurements used for calibration
#define INTERVAL4CAL 3.0f               // 3s at 25Hz: 4 element interval (samples)
#define INTERVAL7CAL 10.0f              // 10s at 25Hz: 7 element interval (samples)
#define INTERVAL10CAL 30.0f             // 30s at 25Hz: 10 element interval (samples)
#define MINBFITUT 10.0F					// minimum acceptable geomagnetic field B (uT) for valid calibration
#define MAXBFITUT 90.0F					// maximum acceptable geomagnetic field B (uT) for valid calibration
#define FITERRORAGINGSECS 86400.0F		// 24 hours: time (s) for fit error to increase (age) by e=2.718
#define MESHDELTA 5.0f                  // magnetic buffer mesh spacing in counts (here 5uT)
#define DEFAULTB 50.0F					// default geomagnetic field (uT)

namespace uquad
{
namespace control
{

    MagnetometerCalibration::MagnetometerCalibration()
    {
        reset();
    }
    
    MagnetometerCalibration::~MagnetometerCalibration()
    {
    }
    
    void MagnetometerCalibration::reset()
    {
        m_bCalibrationHasRun = false;
        
        m_Time = 0.0f;
        m_4InvTime = 0.0f;
        m_7EigTime = 0.0f;
        m_10EigTime = 0.0f;
        
        m_SampleSequence = 0;
        
        m_BufferCount = 0;
        for(std::size_t j = 0; j < MAGBUFFSIZEX; j++)
        {
            for(std::size_t k = 0; k < MAGBUFFSIZEY; k++)
            {
                m_BufferIndex[j][k] = 0;
            }
        }
        
        for(std::size_t j = 0; j < (MAGBUFFSIZEX - 1); j++)
        {
            m_BufferTanArray[j] = static_cast<int>(100.0f * std::tan(M_PI * (-0.5f + static_cast<float>(j + 1) / MAGBUFFSIZEX)));
        }
        
        m_Solver = kSolverNo;
        m_V.setZero();
        m_InvW.setIdentity();
        m_B = DEFAULTB;
        m_FitError = 1000.0F;
        
        m_TrV.setZero();
        m_TrInvW.setIdentity();
        m_TrB = 0.0f;
        m_TrFitError = 0.0f;
        
        m_MapTransform.setIdentity();
        m_CalibrationTransform.setIdentity();
    }
    
    bool MagnetometerCalibration::update(Vec3f const &sample, float dt)
    {
        if(++m_SampleSequence == 0)
            m_SampleSequence = 1;
        
        m_Time += dt;
        m_Sample = sample;
        
        updateBuffer();
        
        m_SampleC = m_CalibrationTransform * m_Sample;
        
        if( (!m_bCalibrationHasRun && (m_BufferCount >= MINMEASUREMENTS4CAL)) ||
            ((m_BufferCount >= MINMEASUREMENTS4CAL) && (m_BufferCount < MINMEASUREMENTS7CAL) && m_Time >= (m_4InvTime + INTERVAL4CAL)) ||
            ((m_BufferCount >= MINMEASUREMENTS7CAL) /*&& (m_BufferCount < MINMEASUREMENTS10CAL)*/ && m_Time >= (m_7EigTime + INTERVAL7CAL))/* ||
            ((m_BufferCount >= MINMEASUREMENTS10CAL) && m_Time >= m_10EigTime) */)
        {
            return updateCalibration();
        }
        
        return false;
    }
    
    float MagnetometerCalibration::calibrationQuality() const
    {
        return std::max(0.0f, std::min(1.0f, 1.0f - m_FitError/100.0f));
    }
    
    Transform3af const& MagnetometerCalibration::mapTransform() const
    {
        return m_MapTransform;
    }
    
    Transform3af const& MagnetometerCalibration::calibrationTransform() const
    {
        return m_CalibrationTransform;
    }
    
    template<class Archive>
    void MagnetometerCalibration::serialize(Archive &ar, const unsigned int)
    {
        ar & m_MapTransform;
        ar & m_CalibrationTransform;
    }
    

    
    void MagnetometerCalibration::updateBuffer()
    {
        if(std::abs(m_SampleC.z()) < 1.0e-3f)
            return;
        
        int tanJ = static_cast<int>(100 * m_SampleC.x()/m_SampleC.z());
        int tanK = static_cast<int>(100 * m_SampleC.y()/m_SampleC.z());
        
        uint64_t binJ = 0;
        uint64_t binK = 0;
        
        while((binJ < (MAGBUFFSIZEX - 1) && (tanJ >= m_BufferTanArray[binJ]))) binJ++;
        while((binK < (MAGBUFFSIZEX - 1) && (tanK >= m_BufferTanArray[binK]))) binK++;
        if(m_SampleC.x() < 0.0f) binK += MAGBUFFSIZEX;
        
        if((m_BufferCount == MAXMEASUREMENTS) && m_BufferIndex[binJ][binK])
        {
            m_BufferBs[binJ][binK] = m_Sample;
            m_BufferIndex[binJ][binK] = m_SampleSequence;
            return;
        }
        
        if((m_BufferCount == MAXMEASUREMENTS) && !m_BufferIndex[binJ][binK])
        {
            m_BufferBs[binJ][binK] = m_Sample;
            m_BufferIndex[binJ][binK] = m_SampleSequence;

            uint64_t i = m_SampleSequence;
            uint64_t l = 1;
            uint64_t m = 1;
            for(std::size_t j = 0; j < MAGBUFFSIZEX; j++)
            {
                for(std::size_t k = 0; k < MAGBUFFSIZEY; k++)
                {
                    if(m_BufferIndex[j][k] < i)
                    {
                        if(m_BufferIndex[j][k])
                        {
                            l = j;
                            m = k;
                            i = m_BufferIndex[l][m];
                        }
                    }
                }
            }
            m_BufferIndex[l][m] = 0;
            return;
        }
        
        if((m_BufferCount < MAXMEASUREMENTS) && !m_BufferIndex[binJ][binK])
        {
            m_BufferBs[binJ][binK] = m_Sample;
            m_BufferIndex[binJ][binK] = m_SampleSequence;
            m_BufferCount++;
            return;
        }
        
        if((m_BufferCount < MAXMEASUREMENTS) && m_BufferIndex[binJ][binK])
        {
            float delta = (m_Sample - m_BufferBs[binJ][binK]).cwiseAbs().sum();
            if(delta < MESHDELTA)
            {
                m_BufferBs[binJ][binK] = m_Sample;
                m_BufferIndex[binJ][binK] = m_SampleSequence;
            }
            else
            {
                bool tooClose = false;
                int64_t l = 1;
                int64_t m = 1;
                
                std::size_t j = 0;
                while(!tooClose && (j < MAGBUFFSIZEX))
                {
                    std::size_t k = 0;
                    while(!tooClose && (k < MAGBUFFSIZEY))
                    {
                        if(m_BufferIndex[j][k])
                        {
                            delta = (m_Sample - m_BufferBs[j][k]).cwiseAbs().sum();
                            if(delta < MESHDELTA)
                            {
                                tooClose = true;
                            }
                        }
                        else
                        {
                            l = j;
                            m = k;
                        }
                        k++;
                    }
                    j++;
                }
                
                if(!tooClose)
                {
                    m_BufferBs[l][m] = m_Sample;
                    m_BufferIndex[l][m] = m_SampleSequence;
                    m_BufferCount++;
                }
            }
            return;
        }
    }
    
    bool MagnetometerCalibration::updateCalibration()
    {
        eSolver solver = kSolverNo;
        
        if(m_BufferCount < MINMEASUREMENTS7CAL)
        {
            if(m_Solver)
            {
                m_FitError *= (1.0f + (float) INTERVAL4CAL / FITERRORAGINGSECS);
            }
            solver = kSolver4Inv;
            updateCalibration4Inv();
            m_4InvTime = m_Time;
        }
        else
        //if(m_BufferCount < MINMEASUREMENTS10CAL)
        {
            if(m_Solver)
            {
                m_FitError *= (1.0f + (float) INTERVAL7CAL / FITERRORAGINGSECS);
            }
            solver = kSolver7Eig;
            updateCalibration7Eig();
            m_7EigTime = m_Time;
        }
        /*
        else
        {
            if(m_Solver)
            {
                m_FitError *= (1.0f + (float) INTERVAL10CAL / FITERRORAGINGSECS);
            }
            solver = kSolver10Eig;
            updateCalibration10Eig();
        }
        */
        
        if((m_TrB >= MINBFITUT) && (m_TrB <= MAXBFITUT))
        {
            if( (m_Solver == kSolverNo) ||
                (m_TrFitError <= m_FitError) ||
                ((solver > m_Solver) && (m_TrFitError <= 4.0f)))
            {
                m_Solver = solver;
                m_FitError = m_TrFitError;
                m_B = m_TrB;
                m_V = m_TrV;
                m_InvW = m_TrInvW;
                m_bCalibrationHasRun = true;
                
                m_CalibrationTransform.setIdentity();
                m_CalibrationTransform.linear() = m_InvW;
                m_CalibrationTransform.translate(-m_V);
                
                
                m_MapTransform.setIdentity();
                m_MapTransform.linear() = m_InvW.inverse();
                m_MapTransform.pretranslate(m_V);
                
                return true;
            }
        }
        
        return false;
    }
    
    void MagnetometerCalibration::updateCalibration4Inv()
    {
        float const scaling = 1.0f/DEFAULTB;
        
        m_TrInvW.setIdentity();
        m_VecB.setZero();
        m_MatA.setZero();
        
        float sumBs4 = 0.0f;
        
        Vec3f offset = Vec3f::Zero();
        std::size_t numMeasurements = 0;
        
        for(std::size_t j = 0; j < MAGBUFFSIZEX; j++)
        {
            for(std::size_t k = 0; k < MAGBUFFSIZEY; k++)
            {
                if(m_BufferIndex[j][k])
                {
                    if(numMeasurements == 0)
                    {
                        offset = m_BufferBs[j][k];
                    }

                    Vec3f const bsDiff = scaling * (m_BufferBs[j][k] - offset);
                    Vec3f const bsDiff2 = bsDiff.cwiseProduct(bsDiff);
                    m_VecA.middleRows<3>(0) = bsDiff;
                    m_VecA.middleRows<3>(3) = bsDiff2;
                    
                    float const Bs2 = bsDiff2.sum();
                    sumBs4 += Bs2 * Bs2;
                    
                    m_VecB.middleRows<3>(0) += Bs2 * bsDiff;
                    m_VecB(3) += Bs2;
                    
                    m_MatA(0, 0) += m_VecA(3);
                    m_MatA(0, 1) += m_VecA(0) * m_VecA(1);
                    m_MatA(0, 2) += m_VecA(0) * m_VecA(2);
                    m_MatA(0, 3) += m_VecA(0);
                    
                    m_MatA(1, 1) += m_VecA(4);
                    m_MatA(1, 2) += m_VecA(1) * m_VecA(2);
                    m_MatA(1, 3) += m_VecA(1);
                    
                    m_MatA(2, 2) += m_VecA(5);
                    m_MatA(2, 3) += m_VecA(2);
                    
                    numMeasurements++;
                }
            }
        }
        
        m_MatA(3, 3) = numMeasurements;
        m_BufferCount = numMeasurements;
        
        for(std::size_t i = 0; i < 4; i++)
        {
            for(std::size_t j = i; j < 4; j++)
            {
                m_MatB(i, j) = m_MatB(j, i) = m_MatA(j, i) = m_MatA(i, j);
            }
        }
        
        m_MatB.block<4, 4>(0, 0) = Mat4x4f(m_MatB.block<4, 4>(0, 0).inverse());
        
        m_VecA.middleRows<4>(0) = m_MatB.block<4, 4>(0, 0) * m_VecB.middleRows<4>(0);
        float E = 0.0f;
        
        E += m_VecA.middleRows<4>(0).dot(m_VecB.middleRows<4>(0));
        E = sumBs4 - 2.0f * E;
        
        m_VecB.middleRows<4>(0) = m_MatA.block<4, 4>(0, 0) * m_VecA.middleRows<4>(0);
        E += m_VecB.middleRows<4>(0).dot(m_VecA.middleRows<4>(0));
        
        m_TrV = 0.5f * m_VecA.middleRows<3>(0);
        
        m_TrB = std::sqrt(m_VecA(3) + m_TrV.dot(m_TrV));
        m_TrFitError  = std::sqrt(E / m_BufferCount) * 100.0f / (2.0f * m_TrB * m_TrB);
        
        m_TrV = m_TrV * DEFAULTB + offset;
        m_TrB *= DEFAULTB;
    }
    
    void MagnetometerCalibration::updateCalibration7Eig()
    {
        float const scaling = 1.0f / DEFAULTB;
        
        Vec3f offset = Vec3f::Zero();
        m_MatA.setZero();
        

        std::size_t numMeasurements = 0;
        for(std::size_t j = 0; j < MAGBUFFSIZEX; j++)
        {
            for(std::size_t k = 0; k < MAGBUFFSIZEY; k++)
            {
                if(m_BufferIndex[j][k])
                {
                    if(numMeasurements == 0)
                    {
                        offset = m_BufferBs[j][k];
                    }
                    
                    Vec3f const bsDiff = scaling * (m_BufferBs[j][k] - offset);
                    Vec3f const bsDiff2 = bsDiff.cwiseProduct(bsDiff);
                    m_VecA.middleRows<3>(3) = bsDiff;
                    m_VecA.middleRows<3>(0) = bsDiff2;
                    
                    for(std::size_t m = 0; m < 6; m++)
                    {
                        m_MatA(m, 6) += m_VecA(m);
                    }
                    
                    for(std::size_t m = 0; m < 6; m++)
                    {
                        for(std::size_t n = m; n < 6; n++)
                        {
                            m_MatA(m, n) += m_VecA(m) * m_VecA(n);
                        }
                    }
                    numMeasurements++;
                }
            }
        }
        
        m_MatA(6, 6) = numMeasurements;
        
        m_BufferCount = numMeasurements;

        for(std::size_t m = 1; m < 7; m++)
        {
            for(std::size_t n = 0; n < m; n++)
            {
                m_MatA(m, n) = m_MatA(n, m);
            }
        }
        
        typedef Matrix<float, 7, 7> Mat7x7f;
        
        Eigen::EigenSolver<Mat7x7f> eigenSolver(m_MatA.block<7, 7>(0, 0));
        
        m_VecA.middleRows<7>(0) = eigenSolver.pseudoEigenvalueMatrix().diagonal();
        m_MatB.block<7, 7>(0, 0) = eigenSolver.pseudoEigenvectors();
        
        std::size_t j = 0;
        for(std::size_t i = 1; i < 7; i++)
        {
            if(m_VecA(i) < m_VecA(j))
            {
                j = i;
            }
        }
        
        Mat3x3f A = Mat3x3f::Zero();
        
        float detA = 1.0f;
        for(std::size_t l = 0; l < 3; l++)
        {
            A(l, l) = m_MatB(l, j);
            detA *= A(l, l);
            m_TrV(l) = -0.5f * m_MatB(l + 3, j) / A(l, l);
        }
        
        if(detA < 0.0f)
        {
            A *= -1.0f;
            m_MatB(6, j) = -m_MatB(6, j);
            detA = -detA;
        }
        
        float tmp = A.diagonal().dot(m_TrV.cwiseProduct(m_TrV));
        tmp -= m_MatB(6, j);
        
        m_TrFitError = 50.0f * std::sqrt(std::abs(m_VecA(j)) / m_BufferCount) / std::abs(tmp);
        
        A *= std::pow(detA, -1.0f/3.0f);
        
        m_TrB = std::sqrt(std::abs(tmp)) * DEFAULTB * std::pow(detA, -1.0f/6.0f);
        
        m_TrInvW.setIdentity();
        
        m_TrInvW.diagonal() = A.diagonal().cwiseAbs().cwiseSqrt();
        
        m_TrV = m_TrV * DEFAULTB + offset;
    }
    
    void MagnetometerCalibration::updateCalibration10Eig()
    {
        float scaling = 1.0f / DEFAULTB;
        
        Vec3f offset = Vec3f::Zero();
        
        m_MatA.setZero();
        
        std::size_t numMeasurements = 0;
        for(std::size_t j = 0; j < MAGBUFFSIZEX; j++)
        {
            for(std::size_t k = 0; k < MAGBUFFSIZEY; k++)
            {
                if(m_BufferIndex[j][k])
                {
                    if(numMeasurements == 0)
                    {
                        offset = m_BufferBs[j][k];
					}
				
                    
                    m_VecA.middleRows<3>(6) = scaling * (m_BufferBs[j][k] - offset);
                    m_VecA(0) = m_VecA(6) * m_VecA(6);
                    m_VecA(1) = 2.0f * m_VecA(6) * m_VecA(7);
                    m_VecA(2) = 2.0f * m_VecA(6) * m_VecA(8);
                    
                    m_VecA(3) = m_VecA(7) * m_VecA(7);
                    m_VecA(4) = 2.0f * m_VecA(7) * m_VecA(8);
                    m_VecA(5) = m_VecA(8) * m_VecA(8);
                    
                    for(std::size_t m = 0; m < 9; m++)
                    {
                        m_MatA(m, 9) += m_VecA(m);
                    }
                
                    for(std::size_t m = 0; m < 9; m++)
                    {
                        for(std::size_t n = m; n < 9; n++)
                        {
                            m_MatA(m, n) += m_VecA(m) * m_VecA(n);
                        }
                    }
                    numMeasurements++;
                }
            }
        }
        
        m_MatA(9, 9) = numMeasurements;
        m_BufferCount = numMeasurements;
        
        for(std::size_t m = 1; m < 10; m++)
        {
            for(std::size_t n = 0; n < m; n++)
            {
                m_MatA(m, n) = m_MatA(n, m);
            }
        }
        
        Eigen::EigenSolver<MatCalib> eigenSolver(m_MatA);
        
        m_VecA = eigenSolver.pseudoEigenvalueMatrix().diagonal();
        m_MatB = eigenSolver.pseudoEigenvectors();
        
        std::size_t j = 0;
        for(std::size_t i = 1; i < 10; i++)
        {
            if(m_VecA(i) < m_VecA(j))
            {
                j = i;
            }
        }
        
        Mat3x3f A = Mat3x3f::Zero();
        
        A(0, 0) = m_MatB(0, j);
        A(0, 1) = A(1, 0) = m_MatB(1, j);
        A(0, 2) = A(2, 0) = m_MatB(2, j);
        A(1, 1) = m_MatB(3, j);
        A(1, 2) = A(2, 1) = m_MatB(4, j);
        A(2, 2) = m_MatB(5, j);
        
        float detA = A.determinant();
        if(detA < 0.0f)
        {
            A *= -1.0f;
            m_MatB(6, j) = -m_MatB(6, j);
            m_MatB(7, j) = -m_MatB(7, j);
            m_MatB(8, j) = -m_MatB(8, j);
            m_MatB(9, j) = -m_MatB(9, j);
            detA = -detA;
        }
        Mat3x3f invA = A.inverse();
        m_TrV = -0.5f * invA * m_MatB.block<3, 1>(6, j);
        
        m_TrB = std::sqrt(
            std::abs(
                A(0, 0) * m_TrV(0) * m_TrV(0) +
                2.0f * A(0, 1) * m_TrV(0) * m_TrV(1) +
                2.0f * A(0, 2) * m_TrV(0) * m_TrV(2) +
                A(1, 1) * m_TrV(1) * m_TrV(1) +
                2.0f * A(1, 2) * m_TrV(1) * m_TrV(2) +
                A(2, 2) * m_TrV(2) * m_TrV(2) - m_MatB(9, j)
            )
        );
        
        m_TrFitError = 50.0f * std::sqrt(std::abs(m_VecA(j)) / m_BufferCount) / (m_TrB * m_TrB);
        
        m_TrV = m_TrV * DEFAULTB - offset;
        m_TrB *= DEFAULTB;
        
        A *= std::pow(detA, -1.0f/3.0f);
        m_TrB *= std::pow(detA, -1.0f/6.0f);
        
        Eigen::EigenSolver<Mat3x3f> eigenSolver3x3(A);
        
        Vec3f eigenValsA = eigenSolver3x3.pseudoEigenvalueMatrix().diagonal();
        Mat3x3f eigenVectorsA = eigenSolver3x3.pseudoEigenvectors();
        
        for(std::size_t j = 0; j < 3; ++j)
        {
            float ftmp = std::sqrt(std::sqrt(std::abs(eigenValsA(j))));
            eigenVectorsA.block<3, 1>(0, j) *= ftmp;
        }
        
        for(std::size_t i = 0; i < 3; i++)
        {
            for(std::size_t j = i; j < 3; j++)
            {
                m_TrInvW(j, i) = m_TrInvW(i, j) = eigenVectorsA.row(i).dot(eigenVectorsA.row(j));
            }
        }
        
    }
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::control::MagnetometerCalibration)
