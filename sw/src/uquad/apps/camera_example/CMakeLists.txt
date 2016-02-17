project(camera_example)

uquad_add_executable(camera_example main.cpp)

install(TARGETS camera_example
	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)

target_link_libraries(camera_example hal boost_system boost_thread boost_chrono)
