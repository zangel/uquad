project(uQuadRC)

uquad_add_executable(uQuadRC
    Application.h
    Application.cpp
	main.cpp
)

target_link_libraries(uQuadRC comm control)

install(TARGETS uQuadRC
    RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
    BUNDLE DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
