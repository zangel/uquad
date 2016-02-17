add_subdirectory(remote)

if(Qt5Widgets_FOUND AND Qt5PrintSupport_FOUND AND OpenCV_FOUND)
	add_subdirectory(local)
endif()
