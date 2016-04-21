set(uquad_control_headers
	Config.h
	Log.h
	Block.h
	BlockLibrary.h
    Signal.h
    System.h
    Clock.h
    Accelerometer.h
    Gyroscope.h
    Magnetometer.h
    Barometer.h
    UQuadSensors.h
    PoseEstimation.h
    AccelerometerCalibration.h
    GyroscopeCalibration.h
    MagnetometerCalibration.h
    Filters.h
    MotorsThrust.h
    QuadMotorsThrust.h
    QuadMotorsThrustSimple.h
    AttitudeControl.h
    AttitudeControlSimple.h
    PositionControl.h
    PositionControlSimple.h
)

set(uquad_control_sources
	Config.cpp
	Block.cpp
	BlockLibrary.cpp
    Signal.cpp
    System.cpp
    Clock.cpp
    Accelerometer.cpp
    Gyroscope.cpp
    Magnetometer.cpp
    Barometer.cpp
    UQuadSensors.cpp
    PoseEstimation.cpp
    AccelerometerCalibration.cpp
    GyroscopeCalibration.cpp
    MagnetometerCalibration.cpp
    MotorsThrust.cpp
    QuadMotorsThrust.cpp
    QuadMotorsThrustSimple.cpp
    AttitudeControl.cpp
    AttitudeControlSimple.cpp
    PositionControl.cpp
    PositionControlSimple.cpp
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
	    ekf2/ecl/EKF/common.h
        ekf2/ecl/EKF/ekf.h
        ekf2/ecl/EKF/estimator_interface.h
        ekf2/ecl/EKF/RingBuffer.h
        ekf2/ecl/EKF/mathlib.cpp
        ekf2/ecl/EKF/geo.cpp
        ekf2/PoseEstimation.h
	)

	set(uquad_control_pose_estimation_ekf2_sources
        ekf2/ecl/EKF/airspeed_fusion.cpp
        ekf2/ecl/EKF/control.cpp
        ekf2/ecl/EKF/covariance.cpp
        ekf2/ecl/EKF/ekf_helper.cpp
        ekf2/ecl/EKF/ekf.cpp
        ekf2/ecl/EKF/estimator_interface.cpp
        ekf2/ecl/EKF/gps_checks.cpp
        ekf2/ecl/EKF/mag_fusion.cpp
        ekf2/ecl/EKF/optflow_fusion.cpp
        ekf2/ecl/EKF/terrain_estimator.cpp
        ekf2/ecl/EKF/vel_pos_fusion.cpp
        ekf2/ecl/EKF/mathlib.h
        ekf2/ecl/EKF/geo.h
		ekf2/PoseEstimation.cpp
	)

	set(uquad_control_pose_estimation_ekf2_libs)

	source_group("ekf2" FILES ${uquad_control_pose_estimation_ekf2_headers} ${uquad_control_pose_estimation_ekf2_sources})

	set(uquad_control_pose_estimation_ekf2_matrix_headers
		ekf2/ecl/matrix/matrix/math.hpp
	    ekf2/ecl/matrix/matrix/helper_functions.hpp
		ekf2/ecl/matrix/matrix/Matrix.hpp
		ekf2/ecl/matrix/matrix/SquareMatrix.hpp
		ekf2/ecl/matrix/matrix/Vector.hpp
		ekf2/ecl/matrix/matrix/Vector2.hpp
		ekf2/ecl/matrix/matrix/Vector3.hpp
		ekf2/ecl/matrix/matrix/Euler.hpp
		ekf2/ecl/matrix/matrix/Dcm.hpp
		ekf2/ecl/matrix/matrix/Scalar.hpp
		ekf2/ecl/matrix/matrix/Quaternion.hpp
	)

	set(uquad_control_pose_estimation_ekf2_matrix_sources
	)

	set(uquad_control_pose_estimation_ekf2_matrix_libs)

	source_group("ekf2\\matrix" FILES ${uquad_control_pose_estimation_ekf2_matrix_headers} ${uquad_control_pose_estimation_ekf2_matrix_sources})

    set(uquad_control_headers ${uquad_control_headers} ${uquad_control_pose_estimation_ekf2_headers} ${uquad_control_pose_estimation_ekf2_matrix_headers})
    set(uquad_control_sources ${uquad_control_sources} ${uquad_control_pose_estimation_ekf2_sources} ${uquad_control_pose_estimation_ekf2_matrix_sources})
    set(uquad_control_libs ${uquad_control_libs} ${uquad_control_pose_estimation_ekf2_libs} ${uquad_control_pose_estimation_ekf2_matrix_libs})
    add_definitions(-DUQUAD_CONTROL_POSE_ESTIMATION_EKF2)
    add_definitions(-DPOSIX_SHARED)
    include_directories(ekf2/ecl)
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
