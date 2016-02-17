
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../extern/libcvd)
include_directories(${EXTERN_DIR}/TooN/include)
include_directories(${EXTERN_DIR}/libcvd)
include_directories(${EXTERN_DIR}/ptam/include)

if(OpenCV_FOUND)
	include_directories(${OpenCV_INCLUDE_DIRS})
	include_directories(${EXTERN_DIR}/sophus/include)
	include_directories(${EXTERN_DIR}/vikit/include)
	include_directories(${EXTERN_DIR}/fast/include)
	include_directories(${EXTERN_DIR}/svo/include)
endif()


set(uquad_cv_headers
	Config.h
	Log.h
    CameraCalibration.h
    Algorithm.h
    AlgorithmLibrary.h
    PoseTracker.h
)

set(uquad_cv_sources
	Config.cpp
    CameraCalibration.cpp
    Algorithm.cpp
    AlgorithmLibrary.cpp
    PoseTracker.cpp
)

set(uquad_cv_libs base)

source_group("" FILES ${uquad_cv_headers} ${uquad_cv_sources})

set(uquad_cv_ptam_headers
    ptam/Context.h
    ptam/PoseTracker.h
)

set(uquad_cv_ptam_sources
    ptam/Context.cpp
    ptam/PoseTracker.cpp
)

set(uquad_cv_ptam_libs ptam)

source_group("ptam" FILES ${uquad_cv_ptam_headers} ${uquad_cv_ptam_sources})


if(OpenCV_FOUND)
	add_definitions(-DUQUAD_CV_HAS_SVO)
	set(uquad_cv_svo_headers
	    svo/PoseTracker.h
	)

	set(uquad_cv_svo_sources
    	svo/PoseTracker.cpp
	)

	set(uquad_cv_svo_libs svo)

	source_group("svo" FILES ${uquad_cv_svo_headers} ${uquad_cv_svo_sources})
	
endif()


add_library(cv
	STATIC
	${uquad_cv_headers} ${uquad_cv_ptam_headers} ${uquad_cv_svo_headers}
	${uquad_cv_sources} ${uquad_cv_ptam_sources} ${uquad_cv_svo_sources}
)

target_link_libraries(cv
	${uquad_cv_libs} ${uquad_cv_ptam_libs} ${uquad_cv_svo_libs}
)

uquad_enable_precompiled_headers(cv Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()