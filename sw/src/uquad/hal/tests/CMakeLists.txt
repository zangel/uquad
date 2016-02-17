set(test_hal_sources
	test.cpp
	testCamera.cpp
	testUQuad.cpp
)

if(UQUAD_PLATFORM_ANDROID OR UQUAD_PLATFORM_MAC)
	set(test_hal_sources
		${test_hal_sources}
		testAndroidCameraParameters.cpp
	)
	
endif()

uquad_add_executable(testHal
    ${test_hal_sources}
)

target_link_libraries(testHal hal base boost_chrono boost_unit_test_framework)


uquad_add_test(Camera testHal)
uquad_add_test(UQuad testHal)


if(UQUAD_PLATFORM_ANDROID OR UQUAD_PLATFORM_MAC)
	uquad_add_test(AndroidCameraParameters testHal)
endif()	