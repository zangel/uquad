# extern/fast/CMakeLists.txt

project(fast)
include(CheckFunctionExists)

include_directories(include)

file(GLOB fast_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/fast/*.h*)
source_group("include\\fast" FILES ${fast_headers})


set(fast_sources
	src/fast_10.cpp
	src/fast_10_score.cpp
	src/nonmax_3x3.cpp
)

if(UQUAD_PLATFORM_IOS OR UQUAD_PLATFORM_ANDROID)
#	list(APPEND fast_sources src/faster_corner_9_neon.cpp)
else()
	list(APPEND fast_sources src/faster_corner_10_sse.cpp)
endif()
source_group("src" FILES ${fast_sources})

set(fast_libs
)

add_library(fast STATIC
	${fast_headers}
	${fast_sources}
)

target_link_libraries(fast
	${fast_libs}
)
