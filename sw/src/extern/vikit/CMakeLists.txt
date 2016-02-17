# extern/vikit/CMakeLists.txt

project(vikit)
include(CheckFunctionExists)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../eigen-3.2.2)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sophus/include)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)

file(GLOB vikit_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/vikit/*.h*)
source_group("include\\vikit" FILES ${vikit_headers})


set(vikit_sources
	src/atan_camera.cpp
    src/omni_camera.cpp
    src/math_utils.cpp
    src/vision.cpp
    src/performance_monitor.cpp
    src/robust_cost.cpp
    src/user_input_thread.cpp
    src/pinhole_camera.cpp
    src/homography.cpp
    src/img_align.cpp
)
source_group("src" FILES ${vikit_sources})

set(vikit_libs
	${OpenCV_LIBS}
)

add_library(vikit STATIC
	${vikit_headers}
	${vikit_sources}
)

target_link_libraries(vikit
	${vikit_libs}
)
