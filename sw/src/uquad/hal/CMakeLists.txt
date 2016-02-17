set(uquad_hal_headers
	Config.h
	Log.h
	Device.h
	DeviceManager.h
	Camera.h
	UQuad.h
	
)

set(uquad_hal_sources
	Config.cpp
	Device.cpp
	DeviceManager.cpp
	Camera.cpp
	UQuad.cpp
)

set(uquad_hal_libs base boost_signals)

source_group("" FILES ${uquad_hal_headers} ${uquad_hal_sources})

if(UQUAD_PLATFORM_ANDROID)
	include_directories(${EXTERN_DIR}/android/libhardware/include)
	include_directories(${EXTERN_DIR}/android/core/include)
	include_directories(${EXTERN_DIR}/android/media/camera/include)
	include_directories(${EXTERN_DIR}/android/native/include)
	include_directories(${EXTERN_DIR}/android/private/include)
	
	
	set(uquad_hal_android_headers
        android/ASharedMemory.h
		android/CameraParameters.h
		android/Camera.h
		android/RawPixelSampleBuffer.h
		android/MetaPixelSampleBuffer.h
		android/IOService.h
		android/USBHostContext.h
		android/UQuad.h
	)
	
	set(uquad_hal_android_sources
        android/ASharedMemory.cpp
        android/CameraParameters.cpp
		android/Camera.cpp
		android/RawPixelSampleBuffer.cpp
        android/MetaPixelSampleBuffer.cpp
        android/IOService.cpp
		android/USBHostContext.cpp
		android/UQuad.cpp
	)
	
	set(uquad_hal_android_libs usb hardware cutils usbhost)
	source_group("android" FILES ${uquad_hal_android_headers} ${uquad_hal_android_sources})
	
	set(uquad_hal_headers
		${uquad_hal_headers}
		${uquad_hal_android_headers}
	)
	
	set(uquad_hal_sources
		${uquad_hal_sources}
		${uquad_hal_android_sources}
	)
	
	set(uquad_hal_libs
		${uquad_hal_libs}
		${uquad_hal_android_libs}
	)
elseif(UQUAD_PLATFORM_MAC)
	
	set(uquad_hal_av_headers
		av/Camera.h
        av/CVImagePixelSampleBuffer.h
	)
	
	set(uquad_hal_av_sources
		av/Camera.mm
        av/CVImagePixelSampleBuffer.mm
	)
	
	set(uquad_hal_av_libs
        "-framework Cocoa"
        "-framework AVFoundation"
        "-framework CoreVideo"
        "-framework CoreMedia"
    )
	source_group("av" FILES ${uquad_hal_av_headers} ${uquad_hal_av_sources})
	
	set(uquad_hal_mac_headers
		mac/Camera.h
        android/CameraParameters.h
	)
	
	set(uquad_hal_mac_sources
		mac/Camera.mm
        android/CameraParameters.cpp
        none/UQuad.cpp
	)
	
	set(uquad_hal_mac_libs
    )
	source_group("mac" FILES ${uquad_hal_mac_headers} ${uquad_hal_mac_sources})

    set(uquad_hal_none_headers
		none/UQuad.h
	)
	
	set(uquad_hal_none_sources
		none/UQuad.cpp
	)
	
	set(uquad_hal_none_libs
    )
    source_group("none" FILES ${uquad_hal_none_headers} ${uquad_hal_none_sources})

	
	set(uquad_hal_headers
		${uquad_hal_headers}
		${uquad_hal_av_headers}
		${uquad_hal_mac_headers}
        ${uquad_hal_none_headers}
	)
	
	set(uquad_hal_sources
		${uquad_hal_sources}
		${uquad_hal_av_sources}
		${uquad_hal_mac_sources}
        ${uquad_hal_none_sources}
	)
	
	set(uquad_hal_libs
		${uquad_hal_libs}
		${uquad_hal_av_libs}
		${uquad_hal_mac_libs}
        ${uquad_hal_none_libs}
	)
elseif(UQUAD_PLATFORM_IOS)
	
	set(uquad_hal_av_headers
		av/Camera.h
        av/CVImagePixelSampleBuffer.h
	)
	
	set(uquad_hal_av_sources
		av/Camera.mm
        av/CVImagePixelSampleBuffer.mm
	)
	
	set(uquad_hal_av_libs
		"-framework Foundation"
        "-framework AVFoundation"
        "-framework CoreVideo"
        "-framework CoreMedia"
    )
	source_group("av" FILES ${uquad_hal_av_headers} ${uquad_hal_av_sources})
	
	set(uquad_hal_ios_headers
		ios/Camera.h
        ios/UQuad.h
	)
	
	set(uquad_hal_ios_sources
		ios/Camera.mm
        ios/UQuad.mm
	)
	
	set(uquad_hal_ios_libs
        "-framework CoreMotion"
    )
	source_group("ios" FILES ${uquad_hal_ios_headers} ${uquad_hal_ios_sources})
	
	set(uquad_hal_headers
		${uquad_hal_headers}
		${uquad_hal_av_headers}
		${uquad_hal_ios_headers}
	)
	
	set(uquad_hal_sources
		${uquad_hal_sources}
		${uquad_hal_av_sources}
		${uquad_hal_ios_sources}
	)
	
	set(uquad_hal_libs
		${uquad_hal_libs}
		${uquad_hal_av_libs}
		${uquad_hal_ios_libs}
	)
endif()


add_library(hal
	STATIC
	${uquad_hal_headers}
	${uquad_hal_sources}
)

target_link_libraries(hal ${uquad_hal_libs})

uquad_enable_precompiled_headers(hal Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()