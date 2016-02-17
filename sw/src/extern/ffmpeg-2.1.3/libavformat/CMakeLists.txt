# extern/ffmpeg-2.1.3/libavformat/CMakeLists.txt

set(avformat_headers
	avformat.h
	avio.h
	version.h
)

set(avformat_libs
	avcodec avutil
)

if(UQUAD_PLATFORM_MAC)

    set(avformat_sources
        adtsenc.c
        allformats.c
        apetag.c
        avc.c
        avio.c
        aviobuf.c
        cutils.c
        format.c
        id3v1.c
        id3v2.c
        isom.c
        latmenc.c
        metadata.c
		mov.c
        mov_chan.c
        movenc.c
        movenchint.c
		mpegts.c
        mpegtsenc.c
        mux.c
        options.c
        os_support.c
        rawenc.c
        riff.c
		riffdec.c
        riffenc.c
        rtp.c
        rtpenc_chain.c
        sdp.c
        seek.c
        url.c
        utils.c
    )

elseif(UQUAD_PLATFORM_ANDROID)

    set(avformat_sources
        adtsenc.c
        allformats.c
        apetag.c
        avc.c
        avio.c
        aviobuf.c
        cutils.c
        format.c
        id3v1.c
        id3v2.c
        isom.c
        latmenc.c
        metadata.c
        mov_chan.c
        movenc.c
        movenchint.c
        mpegtsenc.c
        mux.c
        options.c
        os_support.c
        rawenc.c
        riff.c
        riffenc.c
        rtp.c
        sdp.c
        seek.c
        url.c
        utils.c
    )

endif()

add_library(avformat STATIC ${avformat_sources} ${avformat_headers})
target_link_libraries(avformat ${avformat_libs})
