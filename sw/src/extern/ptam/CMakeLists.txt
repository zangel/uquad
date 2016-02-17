# extern/ptam/CMakeLists.txt

project(ptam)
include(CheckFunctionExists)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ptam)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TooN/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcvd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../agast/include)
include_directories(${PROJECT_BINARY_DIR}/../libcvd)


set(ptam_headers
    include/ptam/Context.h
	include/ptam/ATANCamera.h
    include/ptam/MapPoint.h
    include/ptam/Map.h
    include/ptam/KeyFrame.h
    include/ptam/Bundle.h
    include/ptam/MEstimator.h
    include/ptam/ShiTomasi.h
    include/ptam/SmallBlurryImage.h
    include/ptam/HomographyInit.h
    include/ptam/MiniPatch.h
    include/ptam/PatchFinder.h
    include/ptam/MapMaker.h
    include/ptam/Relocaliser.h
    include/ptam/Tracker.h
    include/ptam/TrackerData.h
    include/ptam/CalibCornerPatch.h
    include/ptam/CalibImage.h
)
source_group("include\\ptam" FILES ${ptam_headers})


set(ptam_sources
	src/ATANCamera.cc
    src/MapPoint.cc
    src/Map.cc
    src/KeyFrame.cc
    src/Bundle.cc
    src/ShiTomasi.cc
    src/SmallBlurryImage.cc
    src/HomographyInit.cc
    src/MiniPatch.cc
    src/PatchFinder.cc
    src/MapMaker.cc
    src/Relocaliser.cc
    src/Tracker.cc
    src/CalibCornerPatch.cc
    src/CalibImage.cc
)
source_group("src" FILES ${ptam_sources})

set(ptam_libs
	cvd
    agast
)

add_library(ptam STATIC
	${ptam_headers}
	${ptam_sources}
)
target_link_libraries(ptam ${ptam_libs})
