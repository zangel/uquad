project(uQuadTool)

SET(QT_USE_QMAIN true)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets)
find_package(Qt5PrintSupport)

set(uQuadTool_headers
    Config.h
    qcustomplot.h
    Application.h
    MainWindow.h
    SensorsAndMotorsView.h
)

set(uQuadTool_sources
    qcustomplot.cpp
    Application.cpp
    MainWindow.cpp
    SensorsAndMotorsView.cpp
    main.cpp
)

set(uQuadTool_ui_files
    MainWindow.ui
    SensorsAndMotorsView.ui
)

set(uQuadTool_ui_files_resource_files
    Resources.qrc
)

qt5_wrap_ui(uQuadTool__generated_ui_headers ${uQuadTool_ui_files})
qt5_add_resources(uQuadTool_generated_resources ${uQuadTool_ui_files_resource_files})

if(UQUAD_PLATFORM_MAC)
	set(uQuad_APP_TYPE MACOSX_BUNDLE)
    set(uQuadTool_sources ${uQuadTool_sources} AppIcon.icns)
    set_source_files_properties(AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(MACOSX_BUNDLE_BUNDLE_NAME "uQuadTool")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set(MACOSX_BUNDLE_COPYRIGHT "uQUad")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.uquad")
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
elseif(UQUAD_PLATFORM_WIN)
	set(uQuad_APP_TYPE WIN32)
	set(uQuadTool_sources ${uQuadTool_sources} Resources.rc)
endif()

add_executable(uQuadTool ${uQuad_APP_TYPE}
    ${uQuadTool_headers}
    ${uQuadTool_sources}
    ${uQuadTool_ui_files}
    ${uQuadTool_generated_ui_headers}
    ${uQuadTool_generated_resources}
)

target_link_libraries(uQuadTool Qt5::Widgets Qt5::PrintSupport)
target_link_libraries(uQuadTool hal boost_system boost_thread boost_chrono)

#install(TARGETS uQuadTool
#	RUNTIME DESTINATION ${UQUAD_EXECUTABLE_OUTPUT_PATH}
#)


