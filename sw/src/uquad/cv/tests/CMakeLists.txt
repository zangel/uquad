set(test_cv_sources
	test.cpp
)

uquad_add_executable(testCV
    ${test_cv_sources}
)

target_link_libraries(testCV cv base boost_unit_test_framework)
