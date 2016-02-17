project(CameraCalibrator)

set(QT_USE_QMAIN true)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CameraCalibrator_headers
    Config.h
    Application.h
    MainWindow.h
	CalibrationView.h
    CameraView.h
)

set(CameraCalibrator_sources
    Application.cpp
    MainWindow.cpp
	CalibrationView.cpp
    CameraView.mm
    main.cpp
)

set(CameraCalibrator_ui_files
    MainWindow.ui
	CalibrationView.ui
)

set(CameraCalibrator_ui_files_resource_files
    Resources.qrc
)

qt5_wrap_ui(CameraCalibrator_generated_ui_headers ${CameraCalibrator_ui_files})
qt5_add_resources(CameraCalibrator_generated_resources ${CameraCalibrator_ui_files_resource_files})

if(UQUAD_PLATFORM_MAC)
	set(CameraCalibrator_APP_TYPE MACOSX_BUNDLE)
    set(CameraCalibrator_sources ${CameraCalibrator_sources} AppIcon.icns)
    set_source_files_properties(AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "CameraCalibrator")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_COPYRIGHT "uQUad")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.uquad")
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
elseif(UQUAD_PLATFORM_WIN)
	set(uQuad_APP_TYPE WIN32)
	set(uQuadTool_sources ${CameraCalibrator_sources} Resources.rc)
endif()

add_executable(CameraCalibrator ${CameraCalibrator_APP_TYPE}
    ${CameraCalibrator_headers}
    ${CameraCalibrator_sources}
    ${CameraCalibrator_ui_files}
    ${CameraCalibrator_generated_ui_headers}
    ${CameraCalibrator_generated_resources}
)

ocv_include_directories()

target_link_libraries(CameraCalibrator Qt5::Widgets)
target_link_libraries(CameraCalibrator comm hal cv "-framework OpenGL")
target_link_libraries(CameraCalibrator opencv_core opencv_calib3d)

#install(TARGETS uQuadTool
#	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
#)


