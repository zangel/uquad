# extern/ffmpeg-2.1.3/libswscale/CMakeLists.txt

set(swscale_headers
	swscale.h
	version.h
)

set(swscale_sources
	input.c
	options.c
	output.c
	rgb2rgb.c
	swscale.c
	swscale_unscaled.c
	utils.c
	yuv2rgb.c
)

set(swscale_libs
	avutil
)

if(UQUAD_PLATFORM_MAC)

	set(swscale_sources
		${swscale_sources}
		x86/rgb2rgb.c
		x86/swscale.c
		x86/yuv2rgb.c
	)

elseif(UQUAD_PLATFORM_ANDROID)

endif()

add_library(swscale STATIC ${swscale_sources} ${swscale_headers})
target_link_libraries(swscale ${swscale_libs})