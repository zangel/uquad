project(example1)

uquad_add_executable(example1 main.cpp)

install(TARGETS example1
	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)

target_link_libraries(example1 hal boost_system boost_thread boost_chrono)
