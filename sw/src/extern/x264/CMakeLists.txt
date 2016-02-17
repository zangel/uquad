# extern/x264/CMakeLists.txt

project(x264 C)

set(x264_headers
	x264.h
)

set(x264_sources
	common/mc.c
	common/predict.c
	common/pixel.c
	common/macroblock.c
	common/frame.c
	common/dct.c
	common/cpu.c
	common/cabac.c
	common/common.c
	common/osdep.c
	common/rectangle.c
	common/set.c
	common/quant.c
	common/deblock.c
	common/vlc.c
	common/mvpred.c
	common/bitstream.c
	encoder/analyse.c
	encoder/me.c
	encoder/ratecontrol.c
	encoder/set.c
	encoder/macroblock.c
	encoder/cabac.c
	encoder/cavlc.c
	encoder/encoder.c
	encoder/lookahead.c
	common/threadpool.c
)

if(UQUAD_PLATFORM_MAC)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.mac ${PROJECT_BINARY_DIR}/config.h @ONLY)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/x264_config.h.mac ${PROJECT_BINARY_DIR}/x264_config.h @ONLY)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

add_library(x264 STATIC ${x264_headers} ${x264_sources})
