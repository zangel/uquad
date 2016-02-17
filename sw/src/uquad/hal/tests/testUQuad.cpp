#include <boost/test/unit_test.hpp>
#include <uquad/hal/DeviceManager.h>
#include <uquad/utility/Singleton.h>
#include <uquad/hal/UQuad.h>

using namespace uquad;
namespace tt = boost::test_tools;

void test_uquad_device(intrusive_ptr<hal::UQuad> const &device)
{
    BOOST_TEST_REQUIRE(!device->isOpen());
    BOOST_TEST_REQUIRE(!device->open());
    BOOST_TEST_REQUIRE(device->isOpen());

    BOOST_TEST_REQUIRE(!device->areMotorsRunning());

    #if 0
    for(int lm = hal::UQuad::kLedMode0; lm < hal::UQuad::kLedModeCount; ++lm)
    {
        hal::UQuad::eLedMode elm = static_cast<hal::UQuad::eLedMode>(lm);
        device->setLedsMode(elm, elm, elm, elm);
        ::usleep(2000000);
    }
    #endif

    BOOST_TEST_REQUIRE(!device->armMotors());

    ::usleep(3000000);

    BOOST_TEST_REQUIRE(device->areMotorsRunning());


    for(int p = 0; p < 50; ++p)
    {
        float mp = p/50.0f;
        device->setMotorsPower(mp, mp, mp, mp);
        ::usleep(50000);
    }
    ::usleep(2000000);

    BOOST_TEST_REQUIRE(!device->disarmMotors());
    ::usleep(500000);

    BOOST_TEST_REQUIRE(!device->areMotorsRunning());

    device->setLedsMode(hal::UQuad::kLedMode0, hal::UQuad::kLedMode0, hal::UQuad::kLedMode0, hal::UQuad::kLedMode0);
    ::usleep(1000000);

    BOOST_TEST_REQUIRE(!device->close());
    BOOST_TEST_REQUIRE(!device->isOpen());
}

BOOST_AUTO_TEST_CASE(UQuad)
{
    SI(hal::DeviceManager).enumerateDevices<hal::UQuad>([](intrusive_ptr<hal::UQuad> const &uquad_dev) -> bool
    {
        BOOST_TEST_MESSAGE("testing " << uquad_dev->name() << " uquad device ...");
        test_uquad_device(uquad_dev);
        BOOST_TEST_MESSAGE("done.");
        return true;
    });
}

/*
#include <termios.h>
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}
*/
