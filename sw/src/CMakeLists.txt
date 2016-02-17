cmake_minimum_required(VERSION 2.8)

project(uquad)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake;")
set(UQUAD_EXECUTABLE_OUTPUT_PATH "${CMAKE_INSTALL_PREFIX}/bin" )

enable_testing()
include(CTest)

SET(BUILD_APPS "YES" CACHE BOOL "Build applications")
	
include(Config)

if(UQUAD_PLATFORM_MAC)
	find_package(Qt5Widgets)
	find_package(Qt5PrintSupport)
	find_package(Qt53DCore)
	find_package(Qt53DRenderer)
	find_package(Qt53DInput)
endif()

if(UQUAD_PLATFORM_MAC OR UQUAD_PLATFORM_IOS)
	find_package(OpenCV NO_DEFAULT_PATH)
endif()

add_subdirectory(extern)
add_subdirectory(uquad)
