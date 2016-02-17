set(test_svo_sources
	test.cpp
    test_pipeline.cpp
    test_live.cpp
)

uquad_add_executable(testSVO
    ${test_svo_sources}
)

target_link_libraries(testSVO svo boost_unit_test_framework)
