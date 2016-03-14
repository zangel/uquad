set(test_control_sources
	test.cpp
    testPoseEstimation.cpp
    testAccelerometerCalibration.cpp
)

uquad_add_executable(testControl
    ${test_control_sources}
)

target_link_libraries(testControl control base boost_unit_test_framework)

uquad_add_test(PoseEstimation testControl)
uquad_add_test(AccelerometerCalibration testControl)