# extern/svo/CMakeLists.txt

project(svo)
include(CheckFunctionExists)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../jpeg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../boost_1_59_0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../eigen-3.2.2)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sophus/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vikit/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../fast/include)
include_directories(include)

add_definitions(-DHAS_JPG)
add_definitions(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)
add_definitions(-DBOOST_CHRONO_VERSION=2)
#add_definitions(-DSVO_DEBUG_OUTPUT)

file(GLOB svo_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/svo/*.h*)
source_group("include\\svo" FILES ${svo_headers})


set(svo_sources
    src/frame_handler_mono.cpp
    src/frame_handler_base.cpp
    src/frame.cpp
    src/point.cpp
    src/map.cpp
    src/pose_optimizer.cpp
    src/initialization.cpp
    src/matcher.cpp
    src/reprojector.cpp
    src/feature_alignment.cpp
    src/feature_detection.cpp
    src/depth_filter.cpp
    src/config.cpp
    src/sparse_img_align.cpp
)
source_group("src" FILES ${svo_sources})

set(svo_libs
	vikit
	fast
	boost_thread
	boost_system
)

add_library(svo STATIC
	${svo_headers}
	${svo_sources}
)

target_link_libraries(svo
	${svo_libs}
)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()