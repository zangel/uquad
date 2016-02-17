project(TrackerRemote)

uquad_add_executable(TrackerRemote
    Application.h
    Application.cpp
    ../common/msg/TrackerInfo.h
    ../common/msg/TrackerInfo.cpp
    main.cpp
)

target_link_libraries(TrackerRemote comm cv)

install(TARGETS TrackerRemote
    RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
    BUNDLE DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
