project(CameraCalibratorRemote)

uquad_add_executable(CameraCalibratorRemote
    Application.h
    Application.cpp
    main.cpp
)

target_link_libraries(CameraCalibratorRemote comm cv)

install(TARGETS CameraCalibratorRemote
    RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
    BUNDLE DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
