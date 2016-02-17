# extern/ffmpeg-2.1.3/libavdevice/CMakeLists.txt

set(avdevice_headers
	avdevice.h
	version.h
)

set(avdevice_sources
	alldevices.c
	avdevice.c
)

set(avdevice_libs
	avformat
	avcodec
	avutil
)

add_library(avdevice STATIC ${avdevice_sources} ${avdevice_headers})
target_link_libraries(avdevice ${avdevice_libs})
