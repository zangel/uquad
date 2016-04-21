#ifndef UQUAD_CONTROL_MAGNETOMETER_CALIBRATION_H
#define UQUAD_CONTROL_MAGNETOMETER_CALIBRATION_H

#include "Config.h"
#include "../base/Serialization.h"

namespace uquad
{
namespace control
{
    class MagnetometerCalibration
    {
    protected:
        enum eSolver
        {
            kSolverNo = 0,
            kSolver4Inv,
            kSolver7Eig,
            kSolver10Eig
        };
        
        typedef Matrix<float, 10, 10> MatCalib;
        typedef Matrix<float, 10, 1> VecCalib;
        
        static std::size_t const MAGBUFFSIZEX = 14;
        static std::size_t const MAGBUFFSIZEY = (2 * MAGBUFFSIZEX);
        
    public:
        
        MagnetometerCalibration();
        ~MagnetometerCalibration();
        
        void reset();
        bool update(Vec3f const &sample, float dt = 20.0e-3f);
        
        float calibrationQuality() const;
        
        Transform3af const& mapTransform() const;
        Transform3af const& calibrationTransform() const;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
        
    private:
        void updateBuffer();
        bool updateCalibration();
        void updateCalibration4Inv();
        void updateCalibration7Eig();
        void updateCalibration10Eig();
        
    protected:
        bool m_bCalibrationHasRun;
        float m_Time;
        float m_4InvTime;
        float m_7EigTime;
        float m_10EigTime;
        uint64_t m_SampleSequence;
        
        Vec3f m_Sample;
        Vec3f m_SampleC;
        
        std::size_t m_BufferCount;
        Vec3f m_BufferBs[MAGBUFFSIZEX][MAGBUFFSIZEY];
        uint64_t m_BufferIndex[MAGBUFFSIZEX][MAGBUFFSIZEY];
        int m_BufferTanArray[MAGBUFFSIZEX - 1];
        
        eSolver m_Solver;
        Vec3f m_V;
        Mat3x3f m_InvW;
        float m_B;
        float m_FitError;
        
        Vec3f m_TrV;
        Mat3x3f m_TrInvW;
        float m_TrB;
        float m_TrFitError;
        
        MatCalib m_MatA;
        MatCalib m_MatB;
        VecCalib m_VecA;
        VecCalib m_VecB;
        
        
        Transform3af m_MapTransform;
        Transform3af m_CalibrationTransform;
    };
    
} //namespace control
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::control::MagnetometerCalibration)

#endif //UQUAD_CONTROL_MAGNETOMETER_CALIBRATION_H
