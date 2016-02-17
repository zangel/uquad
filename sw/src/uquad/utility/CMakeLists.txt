set(uquad_utility_headers
	Config.h
    Singleton.h
    HashExtensions.h
    Constant.h
    RunService.h
    TypeIndexExtractor.h
    EigenMatrixSerialization.h
)

set(uquad_utility_sources
	Config.cpp
)

set(uquad_utility_libs boost_chrono)

source_group("" FILES ${uquad_utility_headers} ${uquad_utility_sources})

add_library(utility
	STATIC
	${uquad_utility_headers}
	${uquad_utility_sources}
)

target_link_libraries(utility ${uquad_utility_libs})

uquad_enable_precompiled_headers(utility Config.h)