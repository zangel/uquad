set(test_control_sources
	test.cpp
    testEKFPoseEstimation.cpp
)

uquad_add_executable(testControl
    ${test_control_sources}
)

target_link_libraries(testControl control base boost_unit_test_framework)

uquad_add_test(EKFPoseEstimation testControl)