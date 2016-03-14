uquad_add_executable(testMath
	test.cpp
	testGeomEllipsoid.cpp
)

target_link_libraries(testMath math boost_unit_test_framework)

uquad_add_test(GeomEllipsoid testMath)
