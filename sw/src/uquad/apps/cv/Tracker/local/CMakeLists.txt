project(Tracker)

set(QT_USE_QMAIN true)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(Tracker_headers
    Config.h
    Application.h
    MainWindow.h
	TrackerView.h
    CameraView.h
    ../common/msg/TrackerInfo.h
)

set(Tracker_sources
    Application.cpp
    MainWindow.cpp
	TrackerView.cpp
    CameraView.mm
    ../common/msg/TrackerInfo.cpp
    main.cpp
)

set(Tracker_ui_files
    MainWindow.ui
	TrackerView.ui
)

set(Tracker_ui_files_resource_files
    Resources.qrc
)

qt5_wrap_ui(Tracker_generated_ui_headers ${Tracker_ui_files})
qt5_add_resources(Tracker_generated_resources ${Tracker_ui_files_resource_files})

if(UQUAD_PLATFORM_MAC)
	set(Tracker_APP_TYPE MACOSX_BUNDLE)
    set(Tracker_sources ${Tracker_sources} AppIcon.icns)
    set_source_files_properties(AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "Tracker")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_COPYRIGHT "uQUad")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.uquad")
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
elseif(UQUAD_PLATFORM_WIN)
	set(uQuad_APP_TYPE WIN32)
	set(uQuadTool_sources ${Tracker_sources} Resources.rc)
endif()

add_executable(Tracker ${Tracker_APP_TYPE}
    ${Tracker_headers}
    ${Tracker_sources}
    ${Tracker_ui_files}
    ${Tracker_generated_ui_headers}
    ${Tracker_generated_resources}
)

target_link_libraries(Tracker Qt5::Widgets)
target_link_libraries(Tracker comm hal cv "-framework OpenGL")

#install(TARGETS uQuadTool
#	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
#)


