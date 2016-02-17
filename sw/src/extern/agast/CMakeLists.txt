project(agast)
include(CheckFunctionExists)

file(GLOB agast_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/agast/*.h)
file(GLOB agast_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/agast)

add_library(agast STATIC
	${agast_headers}
	${agast_sources}
)
