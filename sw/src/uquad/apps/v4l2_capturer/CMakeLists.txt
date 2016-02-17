project(v4l2_capturer)

uquad_add_executable(v4l2_capturer capture.c util.c video_device.c yuv.c main.c)
target_link_libraries(v4l2_capturer log)

install(TARGETS v4l2_capturer
	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)



