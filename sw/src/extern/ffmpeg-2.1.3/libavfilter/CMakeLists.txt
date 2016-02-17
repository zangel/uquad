# extern/ffmpeg-2.1.3/libavfilter/CMakeLists.txt

set(avfilter_headers
	asrc_abuffer.h
	avcodec.h
	avfilter.h
	avfiltergraph.h
	buffersink.h
	buffersrc.h
	version.h
)

set(avfilter_sources
	allfilters.c
	audio.c
	avcodec.c
	avfilter.c
	avfiltergraph.c
	buffer.c
	buffersink.c
	buffersrc.c
	drawutils.c
	fifo.c
	formats.c
	graphdump.c
	graphparser.c
	lavfutils.c
	lswsutils.c
	opencl_allkernels.c
	pthread.c
	transform.c
	video.c
)

set(avfilter_libs
	avutil
	swresample
	avformat
	avcodec
)

add_library(avfilter STATIC ${avfilter_sources} ${avfilter_headers})
target_link_libraries(avfilter ${avfilter_libs})
