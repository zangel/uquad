
set(uquad_base_headers
	Config.h
    ChronoIO.h
    IntrusivePtrIO.h
	RefCounted.h
	Error.h
    Log.h
    Serialization.h
    Time.h
    #YieldContext.h
    YieldEvent.h
	FileHandle.h
    IOStream.h
    IOFileStream.h
    IOSocketStream.h
    Buffer.h
    SampleBuffer.h
    PixelBuffer.h
    PixelSampleBuffer.h
	Memory.h
    MemoryBuffer.h
	FileMappingMemory.h
    Runloop.h
    ImageFormat.h
	PixelFormat.h
    PixelTraits.h
	Delegate.h
	Image.h
    Application.h
    Object.h
    ObjectFactory.h
    Graph.h
)

set(uquad_base_sources
	Config.cpp
	RefCounted.cpp
	Error.cpp
    Log.cpp
    Serialization.cpp
    #YieldContext.cpp
    YieldEvent.cpp
	FileHandle.cpp
    IOStream.cpp
    IOFileStream.cpp
    IOSocketStream.cpp
    Buffer.cpp
    SampleBuffer.cpp
    PixelBuffer.cpp
    PixelSampleBuffer.cpp
	Memory.cpp
    MemoryBuffer.cpp
	FileMappingMemory.cpp
    Runloop.cpp
	Image.cpp
    Application.cpp
    Object.cpp
    ObjectFactory.cpp
    Graph.cpp
)

set(uquad_base_libs boost_thread boost_coroutine boost_system boost_filesystem boost_chrono boost_program_options boost_serialization jpeg)

source_group("" FILES ${uquad_base_headers} ${uquad_base_sources})

if(UQUAD_PLATFORM_ANDROID)
    set(uquad_base_android_headers
        android/Application.h
    )
    
    set(uquad_base_android_sources
        android/Application.cpp
    )
    
    set(uquad_base_android_libs )
    source_group("android" FILES ${uquad_base_android_headers} ${uquad_base_android_sources})
    
    set(uquad_base_headers
        ${uquad_base_headers}
        ${uquad_base_android_headers}
    )
    
    set(uquad_base_sources
        ${uquad_base_sources}
        ${uquad_base_android_sources}
    )
    
    set(uquad_base_libs
        ${uquad_base_libs}
        ${uquad_base_android_libs}
    )
    
elseif(UQUAD_PLATFORM_MAC)
	set(uquad_base_mac_headers
        mac/Application.h
	)
	
	set(uquad_base_mac_sources
        mac/Application.mm
	)
	
	set(uquad_base_mac_libs )
	source_group("mac" FILES ${uquad_base_mac_headers} ${uquad_base_mac_sources})
	
	set(uquad_base_headers
		${uquad_base_headers}
		${uquad_base_mac_headers}
	)
	
	set(uquad_base_sources
		${uquad_base_sources}
		${uquad_base_mac_sources}
	)
	
	set(uquad_base_libs
		${uquad_base_libs}
		${uquad_base_ios_libs}
	)

elseif(UQUAD_PLATFORM_IOS)
	set(uquad_base_ios_headers
        ios/Application.h
	)
	
	set(uquad_base_ios_sources
        ios/Application.mm
	)
	
	set(uquad_base_ios_libs )
	source_group("ios" FILES ${uquad_base_ios_headers} ${uquad_base_ios_sources})
	
	set(uquad_base_headers
		${uquad_base_headers}
		${uquad_base_ios_headers}
	)
	
	set(uquad_base_sources
		${uquad_base_sources}
		${uquad_base_ios_sources}
	)
	
	set(uquad_base_libs
		${uquad_base_libs}
		${uquad_base_ios_libs}
	)
endif()


add_library(base
	STATIC
	${uquad_base_headers} ${uquad_base_sources}
)

target_link_libraries(base ${uquad_base_libs})

uquad_enable_precompiled_headers(base Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()