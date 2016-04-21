set(test_control_sources
	test.cpp
    testSystem.cpp
    testPoseEstimation.cpp
    testAccelerometerCalibration.cpp
    testGyroscopeCalibration.cpp
    testMagnetometerCalibration.cpp
    testMotorsThrust.cpp
    testAttitudeControl.cpp
    testPositionControl.cpp
)

uquad_add_executable(testControl
    ${test_control_sources}
)

target_link_libraries(testControl control base boost_unit_test_framework)

uquad_add_test(PoseSystem testControl)
uquad_add_test(PoseEstimation testControl)
uquad_add_test(AccelerometerCalibration testControl)
uquad_add_test(GyroscopeCalibration testControl)
uquad_add_test(MagnetometerCalibration testControl)
uquad_add_test(MotorsThrust testControl)
uquad_add_test(AttitudeControl testControl)
uquad_add_test(PositionControl testControl)
