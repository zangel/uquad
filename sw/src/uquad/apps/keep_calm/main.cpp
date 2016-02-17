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
	uquad::hal::UQuadDevice uQuadDevice;

	if(uQuadDevice.open())
    {
        cout << "failed to open uQuad device!" << endl;
        return -101;
    }

    uQuadDevice.start();
    
    uQuadDevice.setLedsMode(
    	uquad::hal::UQuadDevice::kLedMode0,
    	uquad::hal::UQuadDevice::kLedMode0,
    	uquad::hal::UQuadDevice::kLedMode0,
    	uquad::hal::UQuadDevice::kLedMode0
    );

    while(true)
    {
    	uQuadDevice.setMotorsPWM(0.16f, 0.16f, 0.16f, 0.16f);
    	uquad::this_thread::sleep_for(uquad::chrono::milliseconds(50));
    	uQuadDevice.setMotorsPWM(0.0f, 0.0f, 0.0f, 0.0f);
    	uquad::this_thread::sleep_for(uquad::chrono::seconds(20));

    }

    uQuadDevice.setMotorsPWM(0.0f, 0.0f, 0.0f, 0.0f);

    uQuadDevice.stop();
    uQuadDevice.close();

	return 0;

}
