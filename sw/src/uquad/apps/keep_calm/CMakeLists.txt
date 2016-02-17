project(keep_calm)

uquad_add_executable(keep_calm main.cpp)
target_link_libraries(keep_calm hal boost_system boost_thread boost_chrono)

install(TARGETS keep_calm
	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
