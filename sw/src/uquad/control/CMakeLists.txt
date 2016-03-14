set(uquad_control_headers
	Config.h
	Log.h
	System.h
	SystemLibrary.h
    Graph.h
	PoseEstimation.h
    AccelerometerCalibration.h
)

set(uquad_control_sources
	Config.cpp
	System.cpp
	SystemLibrary.cpp
    Graph.cpp
	PoseEstimation.cpp
    AccelerometerCalibration.cpp
)

set(uquad_control_libs base)

source_group("" FILES ${uquad_control_headers} ${uquad_control_sources})

set(UQUAD_CONTROL_POSE_ESTIMATION_EKF "YES" CACHE BOOL "Build ekf pose estimation control")
if(UQUAD_CONTROL_POSE_ESTIMATION_EKF)
	set(uquad_control_pose_estimation_ekf_headers
		ekf/estimator_utilities.h
		ekf/estimator_22states.h
	    ekf/PoseEstimation.h
	)

	set(uquad_control_pose_estimation_ekf_sources
		ekf/estimator_utilities.cpp
		ekf/estimator_22states.cpp
		ekf/PoseEstimation.cpp
	)

	set(uquad_control_pose_estimation_ekf_libs)

	source_group("ekf" FILES ${uquad_control_pose_estimation_ekf_headers} ${uquad_control_pose_estimation_ekf_sources})

    set(uquad_control_headers ${uquad_control_headers} ${uquad_control_pose_estimation_ekf_headers})
    set(uquad_control_sources ${uquad_control_sources} ${uquad_control_pose_estimation_ekf_sources})
    set(uquad_control_libs ${uquad_control_libs} ${uquad_control_pose_estimation_ekf_libs})
    add_definitions(-DUQUAD_CONTROL_POSE_ESTIMATION_EKF)
endif()

set(UQUAD_CONTROL_POSE_ESTIMATION_EKF2 "YES" CACHE BOOL "Build ekf2 pose estimation")
if(UQUAD_CONTROL_POSE_ESTIMATION_EKF2)
	set(uquad_control_pose_estimation_ekf2_headers
	    ekf2/RingBuffer.h
	    ekf2/Types.h
	    ekf2/EKFIf.h
	    ekf2/EKF.h
	    ekf2/PoseEstimation.h
	)

	set(uquad_control_pose_estimation_ekf2_sources
	    ekf2/EKFIf.cpp
	    ekf2/EKF.cpp
		ekf2/PoseEstimation.cpp
	)

	set(uquad_control_pose_estimation_ekf2_libs)

	source_group("ekf2" FILES ${uquad_control_pose_estimation_ekf2_headers} ${uquad_control_pose_estimation_ekf2_sources})

	set(uquad_control_pose_estimation_ekf2_matrix_headers
		ekf2/matrix/math.hpp
	    ekf2/matrix/helper_functions.hpp
		ekf2/matrix/Matrix.hpp
		ekf2/matrix/SquareMatrix.hpp
		ekf2/matrix/Vector.hpp
		ekf2/matrix/Vector2.hpp
		ekf2/matrix/Vector3.hpp
		ekf2/matrix/Euler.hpp
		ekf2/matrix/Dcm.hpp
		ekf2/matrix/Scalar.hpp
		ekf2/matrix/Quaternion.hpp
	)

	set(uquad_control_pose_estimation_ekf2_matrix_sources
	)

	set(uquad_control_pose_estimation_ekf2_matrix_libs)

	source_group("ekf2\\matrix" FILES ${uquad_control_pose_estimation_ekf2_matrix_headers} ${uquad_control_pose_estimation_ekf2_matrix_sources})

    set(uquad_control_headers ${uquad_control_headers} ${uquad_control_pose_estimation_ekf2_headers} ${uquad_control_pose_estimation_ekf2_matrix_headers})
    set(uquad_control_sources ${uquad_control_sources} ${uquad_control_pose_estimation_ekf2_sources} ${uquad_control_pose_estimation_ekf2_matrix_sources})
    set(uquad_control_libs ${uquad_control_libs} ${uquad_control_pose_estimation_ekf2_libs} ${uquad_control_pose_estimation_ekf2_matrix_libs})
    add_definitions(-DUQUAD_CONTROL_POSE_ESTIMATION_EKF2)
endif()

set(UQUAD_CONTROL_POSE_ESTIMATION_IKF "YES" CACHE BOOL "Build ikf pose estimation")
if(UQUAD_CONTROL_POSE_ESTIMATION_IKF)
	set(uquad_control_pose_estimation_ikf_headers
		ikf/build.h
		ikf/types.h
		ikf/approximations.h
		ikf/orientation.h
		ikf/matrix.h
		ikf/magnetic.h
#		ikf/mqx_tasks.h
		ikf/fusion.h
	    ikf/PoseEstimation.h
	)

	set(uquad_control_pose_estimation_ikf_sources
		ikf/approximations.cpp
		ikf/orientation.cpp
		ikf/matrix.cpp
		ikf/magnetic.cpp
#		ikf/mqx_tasks.cpp
		ikf/fusion.cpp
		ikf/PoseEstimation.cpp
	)

	set(uquad_control_pose_estimation_ikf_libs)

	source_group("ikf" FILES ${uquad_control_pose_estimation_ikf_headers} ${uquad_control_pose_estimation_ikf_sources})

    set(uquad_control_headers ${uquad_control_headers} ${uquad_control_pose_estimation_ikf_headers})
    set(uquad_control_sources ${uquad_control_sources} ${uquad_control_pose_estimation_ikf_sources})
    set(uquad_control_libs ${uquad_control_libs} ${uquad_control_pose_estimation_ikf_libs})
    add_definitions(-DUQUAD_CONTROL_POSE_ESTIMATION_IKF)
endif()

set(UQUAD_CONTROL_POSE_ESTIMATION_MADGWICK "YES" CACHE BOOL "Build madgwick pose estimation")
if(UQUAD_CONTROL_POSE_ESTIMATION_MADGWICK)
	set(uquad_control_pose_estimation_madgwick_headers
        madgwick/imu_filter.h
        madgwick/PoseEstimation.h
	)

	set(uquad_control_pose_estimation_madgwick_sources
        madgwick/imu_filter.cpp
		madgwick/PoseEstimation.cpp
	)

	set(uquad_control_pose_estimation_madgwick_libs)

	source_group("madgwick" FILES ${uquad_control_pose_estimation_madgwick_headers} ${uquad_control_pose_estimation_madgwick_sources})

    set(uquad_control_headers ${uquad_control_headers} ${uquad_control_pose_estimation_madgwick_headers})
    set(uquad_control_sources ${uquad_control_sources} ${uquad_control_pose_estimation_madgwick_sources})
    set(uquad_control_libs ${uquad_control_libs} ${uquad_control_pose_estimation_madgwick_libs})
    add_definitions(-DUQUAD_CONTROL_POSE_ESTIMATION_MADGWICK)
endif()

add_library(control
	STATIC
	${uquad_control_headers}
    ${uquad_control_sources}
)

target_link_libraries(control
	${uquad_control_libs}
)

uquad_enable_precompiled_headers(control Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
