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

add_library(math
	STATIC
	${uquad_math_headers}
	${uquad_math_sources}
)

target_link_libraries(math ${uquad_math_libs})