#include "UQuad.h"
#include "../DeviceManager.h"
#include "../Log.h"
#include "../../base/Error.h"
#include <CoreMotion/CoreMotion.h>

namespace uquad
{
namespace hal
{
    template<>
    system::error_code Device::registerDevices<UQuad>(DeviceManager &dm)
    {
        dm.registerDevice(
            intrusive_ptr<UQuad>(new ios::UQuad())
        );

        return base::makeErrorCode(base::kENoError);
    }

namespace ios
{
    UQuad::UQuad()
        : hal::UQuad()
        , m_CMSensorsReadyFlags(0)
        , m_OperationQueue(nil)
        , m_MotionManager(nil)
        , m_Altimeter(nil)
        , m_SensorsDataGuard()
        , m_SensorsData()
    {
    }

    UQuad::~UQuad()
    {
        close();
    }

    std::string const& UQuad::name() const
    {
        static std::string _name = "UQuad";
        return _name;
    }

    bool UQuad::isOpen() const
    {
        return m_OperationQueue != nil;
    }

    system::error_code UQuad::open()
    {
        if(isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::open: already opened!";
            return base::makeErrorCode(base::kEAlreadyOpened);
        }
        
        m_OperationQueue = [[NSOperationQueue alloc] init];
        [m_OperationQueue setMaxConcurrentOperationCount:1];
        m_MotionManager = [[CMMotionManager alloc] init];
        
        m_MotionManager.accelerometerUpdateInterval = 0.01;
        m_MotionManager.gyroUpdateInterval = 0.01;
        m_MotionManager.magnetometerUpdateInterval = 0.01;
        
        m_CMSensorsReadyFlags = 0;
        
        [m_MotionManager
            startAccelerometerUpdatesToQueue:m_OperationQueue
            withHandler:^(CMAccelerometerData *accelerometerData, NSError *error)
            {
                m_CMSensorsReadyFlags |= 0x01;
                m_CMSensorsData.velocityRate = Vec3f
                (
                    -accelerometerData.acceleration.x,
                    -accelerometerData.acceleration.y,
                    -accelerometerData.acceleration.z
                );
                handleCMSensors();
            }
        ];
        [m_MotionManager
            startGyroUpdatesToQueue:m_OperationQueue
            withHandler:^(CMGyroData *gyroData, NSError *error)
            {
                m_CMSensorsReadyFlags |= 0x02;
                m_CMSensorsData.rotationRate = Vec3f
                (
                    gyroData.rotationRate.x,
                    gyroData.rotationRate.y,
                    gyroData.rotationRate.z
                );
                handleCMSensors();
            }
        ];
        [m_MotionManager
            startMagnetometerUpdatesToQueue:m_OperationQueue
            withHandler:^(CMMagnetometerData *magnetometerData, NSError *error)
            {
                m_CMSensorsReadyFlags |= 0x04;
                m_CMSensorsData.magneticField = Vec3f
                (
                     -magnetometerData.magneticField.x,
                     -magnetometerData.magneticField.y,
                     -magnetometerData.magneticField.z
                );
                handleCMSensors();
            }
        ];
        
        if([CMAltimeter isRelativeAltitudeAvailable])
        {
            m_Altimeter = [[CMAltimeter alloc] init];
            [m_Altimeter
                startRelativeAltitudeUpdatesToQueue:m_OperationQueue
                withHandler:^(CMAltitudeData *altitudeData, NSError *error)
                {
                    m_CMSensorsReadyFlags |= 0x08;
                    m_CMSensorsData.baroPressure = altitudeData.pressure.floatValue;
                    //m_CMSensorsData.baroPressure = altitudeData.relativeAltitude.floatValue;
                }
            ];
        }
        
        notifyOnDeviceOpened();
        UQUAD_HAL_LOG(Debug) << "UQuad::open: uquad device opened!";
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::close()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::close: uquad device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }
        
        if(m_Altimeter)
        {
            [m_Altimeter stopRelativeAltitudeUpdates];
            [m_Altimeter release];
            m_Altimeter = nil;
        }
        
        [m_MotionManager stopMagnetometerUpdates];
        [m_MotionManager stopGyroUpdates];
        [m_MotionManager stopMagnetometerUpdates];
        
        [m_MotionManager release];
        m_MotionManager = nil;
        
        [m_OperationQueue release];
        m_OperationQueue = nil;
        
        notifyOnDeviceClosed();
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::getSensorsData(UQuadSensorsData &sd) const
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::getSensorsData: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        lock_guard<fast_mutex> lock(m_SensorsDataGuard);
        sd = m_SensorsData;
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3)
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setLedsMode: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }
        
        return base::makeErrorCode(base::kENotApplicable);
    }

    system::error_code UQuad::armMotors()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setLedsMode: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }
        
        return base::makeErrorCode(base::kENotApplicable);
    }

    bool UQuad::areMotorsRunning() const
    {
        return false;
    }

    system::error_code UQuad::disarmMotors()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setLedsMode: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }
        
        return base::makeErrorCode(base::kENotApplicable);
    }

    system::error_code UQuad::setMotorsPower(float mpfl, float mpfr, float mprl, float mprr)
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setMotorsPower: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }
        
        return base::makeErrorCode(base::kENotApplicable);
    }
    
    void UQuad::handleCMSensors()
    {
        if((m_CMSensorsReadyFlags & 0x07) == 0x07)
        {
            {
                lock_guard<fast_mutex> lock(m_SensorsDataGuard);
                m_SensorsData.time = base::TimeClock::now();
                m_SensorsData.velocityRate = m_CMSensorsData.velocityRate;
                m_SensorsData.rotationRate = m_CMSensorsData.rotationRate;
                m_SensorsData.magneticField = m_CMSensorsData.magneticField;
            }
            
            if(m_CMSensorsReadyFlags & 0x08)
            {
                m_SensorsData.baroPressure = m_CMSensorsData.baroPressure;
                notifyOnUQuadSensorsReady();
            }
            
            m_CMSensorsReadyFlags &= ~ 0x07;
        }
    }

} //namespace ios
} //namespace hal
} //namespace uquad
