set(uquad_media_headers
    Config.h
    Log.h
    CodecType.h
    SampleType.h
    Sample.h
    CodecProfile.h
    ContainerFormat.h
    Component.h
    Port.h
    Source.h
    Sink.h
    Generator.h
    Consumer.h
    Codec.h
    VideoCodec.h
    VideoEncoder.h
    VideoDecoder.h
    Capture.h
    Reader.h
    Writer.h
    Graph.h
    ComponentFactory.h
    SampleFormat.h
    VideoSampleFormat.h
    RawReader.h
    RawWriter.h
    Playout.h
)

set(uquad_media_sources
    Config.cpp
    Component.cpp
    Port.cpp
    Source.cpp
    Sink.cpp
    Generator.cpp
    Consumer.cpp
    Codec.cpp
    VideoCodec.cpp
    VideoEncoder.cpp
    VideoDecoder.cpp
    Capture.cpp
    Reader.cpp
    Writer.cpp
    Graph.cpp
    ComponentFactory.cpp
    SampleFormat.cpp
    VideoSampleFormat.cpp
    RawReader.cpp
    RawWriter.cpp
    Playout.cpp
)

set(uquad_media_libs base hal boost_thread boost_system boost_filesystem)

source_group("" FILES ${uquad_media_headers} ${uquad_media_sources})

if(UQUAD_PLATFORM_ANDROID)

    include_directories(${EXTERN_DIR}/android/libhardware/include)
    include_directories(${EXTERN_DIR}/android/core/include)
    include_directories(${EXTERN_DIR}/android/media/camera/include)
    include_directories(${EXTERN_DIR}/android/native/include)
    include_directories(${EXTERN_DIR}/android/private/include)
    include_directories(${EXTERN_DIR}/android/omxcore/include)

    set(uquad_media_omx_headers
        omx/OMXCore.h
        omx/VideoEncoder.h
        omx/VideoDecoder.h
    )
    
    set(uquad_media_omx_sources
        omx/OMXCore.cpp
        omx/VideoEncoder.cpp
        omx/VideoDecoder.cpp
    )
    
    set(uquad_media_omx_libs OmxCore)
    source_group("omx" FILES ${uquad_media_omx_headers} ${uquad_media_omx_sources})
    
    set(uquad_media_android_headers
        android/VideoEncoder.h
        android/VideoDecoder.h
        android/Reader.h
        android/Writer.h
    )
    
    set(uquad_media_android_sources
        android/VideoEncoder.cpp
        android/VideoDecoder.cpp
        android/Reader.cpp
        android/Writer.cpp
    )
    
    set(uquad_media_android_libs)
    source_group("android" FILES ${uquad_media_android_headers} ${uquad_media_android_sources})
    
    
    set(uquad_media_headers
        ${uquad_media_headers}
        ${uquad_media_omx_headers}
        ${uquad_media_android_headers}
    )
    
    set(uquad_media_sources
        ${uquad_media_sources}
        ${uquad_media_omx_sources}
        ${uquad_media_android_sources}
    )
    
    set(uquad_media_libs
        ${uquad_media_libs}
        ${uquad_media_omx_libs}
        ${uquad_media_android_libs}
    )

elseif(UQUAD_PLATFORM_MAC)

#    include_directories(${EXTERN_DIR}/ffmpeg-2.1.3)
#    include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../extern/ffmpeg-2.1.3)

#    set(uquad_media_ffmpeg_headers
#        ffmpeg/FFMpeg.h
#        ffmpeg/AVFramePixelSampleBuffer.h
#        ffmpeg/AVPacketSampleBuffer.h
#        ffmpeg/VideoEncoder.h
#        ffmpeg/VideoDecoder.h
#        ffmpeg/Writer.h
#    )
    
#    set(uquad_media_ffmpeg_sources
#        ffmpeg/FFMpeg.cpp
#        ffmpeg/AVFramePixelSampleBuffer.cpp
#        ffmpeg/AVPacketSampleBuffer.cpp
#        ffmpeg/VideoEncoder.cpp
#        ffmpeg/VideoDecoder.cpp
#        ffmpeg/Writer.cpp
#    )
    
#    set(uquad_media_ffmpeg_libs avformat avcodec swscale)
#    source_group("ffmpeg" FILES ${uquad_media_ffmpeg_headers} ${uquad_media_ffmpeg_sources})

    set(uquad_media_vt_headers
        vt/VT.h
        vt/H264VideoEncoder.h
        vt/H264VideoDecoder.h
    )
    
    set(uquad_media_vt_sources
        vt/VT.mm
        vt/H264VideoEncoder.mm
        vt/H264VideoDecoder.mm
    )
    
    set(uquad_media_vt_libs
        "-framework AVFoundation"
        "-framework CoreVideo"
        "-framework CoreMedia"
        "-framework VideoToolbox"
    )
    source_group("vt" FILES ${uquad_media_vt_headers} ${uquad_media_vt_sources})


    set(uquad_media_mac_headers
        mac/VideoEncoder.h
        mac/VideoDecoder.h
        mac/Reader.h
        mac/Writer.h
    )
    
    set(uquad_media_mac_sources
        mac/VideoEncoder.cpp
        mac/VideoDecoder.cpp
        mac/Reader.cpp
        mac/Writer.cpp
    )
    
    set(uquad_media_mac_libs)
    source_group("mac" FILES ${uquad_media_mac_headers} ${uquad_media_mac_sources})
    
    
    set(uquad_media_headers
        ${uquad_media_headers}
 #       ${uquad_media_ffmpeg_headers}
        ${uquad_media_vt_headers}
        ${uquad_media_mac_headers}
    )
    
    set(uquad_media_sources
        ${uquad_media_sources}
 #       ${uquad_media_ffmpeg_sources}
        ${uquad_media_vt_sources}
        ${uquad_media_mac_sources}
    )
    
    set(uquad_media_libs
        ${uquad_media_libs}
 #       ${uquad_media_ffmpeg_libs}
        ${uquad_media_vt_libs}
        ${uquad_media_mac_libs}
    )
elseif(UQUAD_PLATFORM_IOS)

    set(uquad_media_vt_headers
        vt/VT.h
        vt/H264VideoEncoder.h
        vt/H264VideoDecoder.h
    )
    
    set(uquad_media_vt_sources
        vt/VT.mm
        vt/H264VideoEncoder.mm
        vt/H264VideoDecoder.mm
    )
    
    set(uquad_media_vt_libs
        "-framework AVFoundation"
        "-framework CoreVideo"
        "-framework CoreMedia"
        "-framework VideoToolbox"
    )
    source_group("vt" FILES ${uquad_media_vt_headers} ${uquad_media_vt_sources})


    set(uquad_media_ios_headers
        mac/VideoEncoder.h
        mac/VideoDecoder.h
        mac/Reader.h
        mac/Writer.h
    )
    
    set(uquad_media_ios_sources
        mac/VideoEncoder.cpp
        mac/VideoDecoder.cpp
        mac/Reader.cpp
        mac/Writer.cpp
    )
    
    set(uquad_media_ios_libs)
    source_group("ios" FILES ${uquad_media_ios_headers} ${uquad_media_ios_sources})
    
    
    set(uquad_media_headers
        ${uquad_media_headers}
        ${uquad_media_vt_headers}
        ${uquad_media_ios_headers}
    )
    
    set(uquad_media_sources
        ${uquad_media_sources}
        ${uquad_media_vt_sources}
        ${uquad_media_ios_sources}
    )
    
    set(uquad_media_libs
        ${uquad_media_libs}
        ${uquad_media_vt_libs}
        ${uquad_media_ios_libs}
    )
endif()

add_library(media
    STATIC
    ${uquad_media_headers} ${uquad_media_sources}
)

target_link_libraries(media ${uquad_media_libs})

uquad_enable_precompiled_headers(media Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()