#include <uquad/Config.h>
#include <uquad/hal/UQuadDevice.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <iostream>

using namespace std;

typedef uquad::accumulators::accumulator_set
<
	float,
		uquad::accumulators::stats
	    <
	    	uquad::accumulators::tag::min,
	    	uquad::accumulators::tag::max,
	    	uquad::accumulators::tag::mean,
	    	uquad::accumulators::tag::variance
	    >
> stats;


int main(int argc, char *argv[])
{
    uquad::hal::UQuadDevice uquadDevice;
    
    if(!uquadDevice.open())
    {
        uquadDevice.start();
        uquadDevice.setMotorsPWM(50.0f, 50.0f, 50.0f, 50.0f);
        uquad::this_thread::sleep_for(uquad::chrono::seconds(2));
        uquadDevice.setMotorsPWM(100.0f, 100.0f, 100.0f, 100.0f);
        uquad::this_thread::sleep_for(uquad::chrono::seconds(1));
        uquadDevice.setMotorsPWM(10.0f, 10.0f, 10.0f, 10.0f);
        uquad::this_thread::sleep_for(uquad::chrono::seconds(2));
        uquadDevice.setMotorsPWM(0.0f, 0.0f, 0.0f, 0.0f);
        
        uquadDevice.m_DataReadySignal.connect([&uquadDevice]()
        {
            cout
                //<< uquadDevice.m_Acceleration
                //<< endl
                << uquadDevice.m_RotationRate
                << endl
                //<< uquadDevice.m_MagneticField
                //<< endl
                //<< uquadDevice.m_AtmTemperature
                << endl
                << uquadDevice.m_AtmPressure
                << endl;
        });
        
        uquad::this_thread::sleep_for(uquad::chrono::seconds(20));
        uquadDevice.stop();
        uquadDevice.close();
    }
        
    /*
    if(!uquadDevice.open())
    {
        cout << "failed to open accelerometer!" << endl;
        return -100;
    }
    
    
    uquad::chrono::milliseconds accInterval;
    if(!accelerometer.getMeasureInterval(accInterval))
    {
    	cout << "failed to get accelerometer measure interval!" << endl;
    	return -101;
    }

    cout << "accelerometer.getInterval=" << accInterval << endl;
    accInterval = uquad::chrono::milliseconds(20);
    if(!accelerometer.setMeasureInterval(accInterval))
    {
    	cout << "failed to set accelerometer measure interval!" << endl;
    	return -102;
    }

    accelerometer.getMeasureInterval(accInterval);
    cout << "accelerometer.measureInterval=" << accInterval << endl;
    
    uquad::hal::Magnetometer magnetometer;
    
    if(!magnetometer.open())
    {
        cout << "failed to open magnetometer!" << endl;
        return -100;
    }
    
    uquad::chrono::milliseconds magInterval;
    if(!magnetometer.setMeasureInterval(magInterval))
    {
    	cout << "failed to get magnetometer measure interval!" << endl;
    	return -101;
    }

    cout << "magnetometer.getInterval=" << magInterval << endl;
    magInterval = uquad::chrono::milliseconds(20);
    if(!magnetometer.setMeasureInterval(magInterval))
    {
    	cout << "failed to set magnetometer measure interval!" << endl;
    	return -102;
    }

    magnetometer.getMeasureInterval(magInterval);
    cout << "magnetometer.measureInterval=" << magInterval << endl;
    
    bool ok = true;
    while(ok)
    {
    	stats accStats[4], magStats[4];
    	for(int i=0;i<50;++i)
    	{
    		uquad::chrono::steady_clock::time_point measureStart = uquad::chrono::steady_clock::now();
			uquad::hal::Accelerometer::Acceleration accData;
			uquad::chrono::steady_clock::time_point accStart = uquad::chrono::steady_clock::now();
			if(!accelerometer.readAcceleration(accData))
			{
				cout << "failed to read magnetometer data!" << endl;
				ok = false;
				break;
			}
			uquad::chrono::microseconds accDuration = uquad::chrono::duration_cast<uquad::chrono::microseconds>(uquad::chrono::steady_clock::now() - accStart);
			accStats[0](accData(0));
			accStats[1](accData(1));
			accStats[2](accData(2));
			accStats[3](static_cast<float>(accDuration.count()));




			uquad::hal::Magnetometer::MagneticField magData;
			uquad::chrono::steady_clock::time_point magStart = uquad::chrono::steady_clock::now();
			if(!magnetometer.readMagneticField(magData))
			{
				cout << "failed to read magnetometer data!" << endl;
				ok = false;
				break;
			}
			uquad::chrono::microseconds magDuration = uquad::chrono::duration_cast<uquad::chrono::microseconds>(uquad::chrono::steady_clock::now() - magStart);
			magStats[0](magData(0));
			magStats[1](magData(1));
			magStats[2](magData(2));
			magStats[3](static_cast<float>(magDuration.count()));

			boost::this_thread::sleep_for(
				uquad::chrono::milliseconds(20) -
				uquad::chrono::duration_cast<uquad::chrono::milliseconds>(uquad::chrono::steady_clock::now() - measureStart)
			);
    	}
    	cout
    		<< "acc: ("
    			<< uquad::accumulators::mean(accStats[0]) << "/" << uquad::accumulators::variance(accStats[0]) << ", "
    			<< uquad::accumulators::mean(accStats[1]) << "/" << uquad::accumulators::variance(accStats[1]) << ", "
    			<< uquad::accumulators::mean(accStats[2]) << "/" << uquad::accumulators::variance(accStats[2])
    			<< "):" << uquad::accumulators::mean(accStats[3]) << "/" << uquad::accumulators::variance(accStats[3])
    			<< endl
			<< "mag: ("
				<< uquad::accumulators::mean(magStats[0]) << "/" << uquad::accumulators::variance(magStats[0]) << ", "
				<< uquad::accumulators::mean(magStats[1]) << "/" << uquad::accumulators::variance(magStats[1]) << ", "
				<< uquad::accumulators::mean(magStats[2]) << "/" << uquad::accumulators::variance(magStats[2])
				<< "):" << uquad::accumulators::mean(magStats[3]) << "/" << uquad::accumulators::variance(magStats[3])
				<< endl;
    }
     */
	return 0;
}
