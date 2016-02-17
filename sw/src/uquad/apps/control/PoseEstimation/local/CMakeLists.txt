project(PoseEstimation)

set(QT_USE_QMAIN true)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(PoseEstimation_headers
    Config.h
    Application.h
    MainWindow.h
	PoseEstimationView.h
    Pose3DView.h
    ../common/msg/UQuadSensorsData.h
	qcustomplot.h
)

set(PoseEstimation_sources
    Application.cpp
    MainWindow.cpp
	PoseEstimationView.cpp
    Pose3DView.cpp
    ../common/msg/UQuadSensorsData.cpp
	qcustomplot.cpp
    main.cpp
)

set(PoseEstimation_ui_files
    MainWindow.ui
	PoseEstimationView.ui
)

set(PoseEstimation_ui_files_resource_files
    Resources.qrc
)

qt5_wrap_ui(PoseEstimation_generated_ui_headers ${PoseEstimation_ui_files})
qt5_add_resources(PoseEstimation_generated_resources ${PoseEstimation_ui_files_resource_files})

if(UQUAD_PLATFORM_MAC)
	set(PoseEstimation_APP_TYPE MACOSX_BUNDLE)
    set(PoseEstimation_sources ${PoseEstimation_sources} AppIcon.icns)
    set_source_files_properties(AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "PoseEstimation")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_COPYRIGHT "uQUad")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.uquad")
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
elseif(UQUAD_PLATFORM_WIN)
	set(uQuad_APP_TYPE WIN32)
	set(PoseEstimation_sources ${PoseEstimation_sources} Resources.rc)
endif()

add_executable(PoseEstimation ${PoseEstimation_APP_TYPE}
    ${PoseEstimation_headers}
    ${PoseEstimation_sources}
    ${PoseEstimation_ui_files}
    ${PoseEstimation_generated_ui_headers}
    ${PoseEstimation_generated_resources}
)

target_link_libraries(PoseEstimation Qt5::Widgets Qt5::PrintSupport Qt5::3DCore Qt5::3DInput Qt5::3DRenderer)
target_link_libraries(PoseEstimation comm hal control)

#install(TARGETS Navigation
#	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
#)


