set(uquad_math_headers
	Config.h
    FixedPoint.h
    Constant.h
    Utils.h
)

set(uquad_math_sources
	Config.cpp
)

set(uquad_math_libs)

source_group("" FILES ${uquad_math_headers} ${uquad_math_sources})

set(uquad_math_geom_headers
	geom/Ellipsoid.h
    geom/ArcBall.h
)

set(uquad_math_geom_sources
    geom/ArcBall.cpp
)

set(uquad_math_geom_libs)

source_group("geom" FILES ${uquad_math_geom_headers} ${uquad_math_geom_sources})


add_library(math
	STATIC
	${uquad_math_headers} ${uquad_math_geom_headers}
	${uquad_math_sources} ${uquad_math_geom_sources}
)

target_link_libraries(math ${uquad_math_libs} ${uquad_math_geom_libs})

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()