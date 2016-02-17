project(camera-server)

uquad_add_executable(camera-server main.cpp)

target_link_libraries(camera-server comm)

install(TARGETS camera-server
    RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
    BUNDLE DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
