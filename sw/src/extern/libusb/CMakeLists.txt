# extern/libusb/CMakeLists.txt

project(libusb C)



set(libusb_headers
	config.h
	version.h
	version_nano.h
	libusb.h
	libusbi.h
	hotplug.h
)

set(libusb_sources
	core.c
	descriptor.c
	hotplug.c
	io.c
	sync.c
	strerror.c
)

set(libusb_os_headers)
set(libusb_os_sources)

set(libusb_libs)


if(UQUAD_PLATFORM_MAC)

    set(libusb_os_headers
        ${libusb_os_headers}
        os/darwin_usb.h
        os/poll_posix.h
        os/threads_posix.h
    )

    set(libusb_os_sources
        ${libusb_os_sources}
        os/darwin_usb.c
        os/poll_posix.c
        os/threads_posix.c
    )

	set(libusb_libs ${libusb_libs}
        objc
        "-framework IOKit"
        "-framework CoreFoundation"
    )
	
elseif(UQUAD_PLATFORM_ANDROID)

    set(libusb_os_headers
        ${libusb_os_headers}
        os/linux_usbfs.h
        os/poll_posix.h
        os/threads_posix.h
    )

    set(libusb_os_sources
        ${libusb_os_sources}
        os/linux_usbfs.c
        os/poll_posix.c
        os/threads_posix.c
        os/linux_netlink.c
    )
	
	set(libusb_libs ${libusb_libs} log)

endif()


include_directories(${PROJECT_SOURCE_DIR})

add_library(usb
    STATIC
    ${libusb_sources}
    ${libusb_headers}
    ${libusb_os_headers}
    ${libusb_os_sources}
)

target_link_libraries(usb ${libusb_libs})