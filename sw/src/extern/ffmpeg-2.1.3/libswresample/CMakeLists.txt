# extern/ffmpeg-2.1.3/libswresample/CMakeLists.txt

set(swresample_headers
	swresample.h
	version.h
)

set(swresample_sources
	audioconvert.c
	dither.c
	rematrix.c
	resample.c
	swresample.c
)

set(swresample_libs
	avutil
)

if(UQUAD_PLATFORM_ANDROID)

	set(swresample_sources
		${swresample_sources}
			arm/audio_convert_init.c
			arm/audio_convert_neon.S
	)
	
endif()

add_library(swresample STATIC ${swresample_sources} ${swresample_headers})
target_link_libraries(swresample ${swresample_libs})
