#include <boost/test/unit_test.hpp>
#include <uquad/control/MagnetometerCalibration.h>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include "../ikf/types.h"
#include "../ikf/magnetic.h"

#define UQUAD_MAG_COUNTSPERUT           6
#define UQUAD_ACC_COUNTSPERG            8192
#define UQUAD_GYR_COUNTSPERDEGPERSEC    64
#define UQUAD_BAR_MPERCOUNT             4
#define UQUAD_BAR_CPERCOUNT             256


using namespace uquad;
namespace tt = boost::test_tools;

typedef boost::minstd_rand base_generator_type;

static std::size_t const NUM_PTS = 1000;

BOOST_AUTO_TEST_CASE(MagnetometerCalibration)
{
    control::MagnetometerCalibration scal;
    
    control::ikf::MagSensor magSensor;
    control::ikf::MagneticBuffer magBuffer;
    control::ikf::MagCalibration magCalibration;
    control::ikf::SV_Base magBase;
    
    
    
    magSensor.iCountsPeruT = UQUAD_MAG_COUNTSPERUT;
    magSensor.fCountsPeruT = (float)UQUAD_MAG_COUNTSPERUT;
    magSensor.fuTPerCount = 1.0F / UQUAD_MAG_COUNTSPERUT;
        
    for(int i = CHX; i <= CHZ; i++)
        magSensor.iBcAvg[i]= 0;
        
    control::ikf::fInitMagCalibration(&magCalibration, &magBuffer);
    magBase.fSensorDT = 20.0e-3f;
    
    
    
    boost::minstd_rand generator(static_cast<unsigned int>(std::time(0)));
    boost::uniform_real<> uni_dist(-1,1);
    boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > uni(generator, uni_dist);
    
    Vec3f center = 50.0f*Vec3f(uni(), uni(), uni()).normalized();
    
    
    Vec3f radii = 50.0f*Vec3f::Ones() + 10.0f*Vec3f(uni(), uni(), uni());
    
    for(std::size_t p = 0; p < NUM_PTS; p++)
    {
        Vec3f pt(uni(), uni(), uni());
        pt.normalize();
        
        float dev = (10 + uni())/10.0f;
        
        Vec3f sample = radii.cwiseProduct(pt);
        
        Vec3f sampleUnc = center + dev * sample;
        
        scal.update(sampleUnc, magBase.fSensorDT);
        
        magSensor.iBs[0] = magSensor.iCountsPeruT*sampleUnc(0);
        magSensor.iBs[1] = magSensor.iCountsPeruT*sampleUnc(1);
        magSensor.iBs[2] = magSensor.iCountsPeruT*sampleUnc(2);

        //mag
        for(int i = CHX; i <= CHZ; i++)
        {
            magSensor.iBsBuffer[i] = magSensor.iBs[i];
        }
        
        iUpdateMagnetometerBuffer(&magBuffer, &magSensor, p);
        
        for(int i = CHX; i <= CHZ; i++)
        {
            magSensor.iBsAvg[i] = magSensor.iBsBuffer[i];
            magSensor.fBsAvg[i] = (float)magSensor.iBsAvg[i] * magSensor.fuTPerCount;
        }
        
        fInvertMagCal(&magSensor, &magCalibration);
        
        if(magBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL)
        {
            magCalibration.iMagCalHasRun = true;
            fRunMagCalibration(&magBase, &magCalibration, &magBuffer, &magSensor);
        }
        
        
        
        
    }
}