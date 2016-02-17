project(PoseEstimationRemote)

uquad_add_executable(PoseEstimationRemote
    Application.h
    Application.cpp
	../common/msg/UQuadSensorsData.h
	../common/msg/UQuadSensorsData.cpp
    main.cpp
)

target_link_libraries(PoseEstimationRemote comm control)

install(TARGETS PoseEstimationRemote
    RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
    BUNDLE DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
)
