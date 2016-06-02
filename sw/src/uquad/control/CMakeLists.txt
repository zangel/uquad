set(uquad_control_headers
	Config.h
	Log.h
	Block.h
	BlockLibrary.h
    Signal.h
    System.h
    AccelerometerCalibration.h
    GyroscopeCalibration.h
    MagnetometerCalibration.h
    Filters.h
)

set(uquad_control_sources
	Config.cpp
	Block.cpp
	BlockLibrary.cpp
    Signal.cpp
    System.cpp
    AccelerometerCalibration.cpp
    GyroscopeCalibration.cpp
    MagnetometerCalibration.cpp
)

set(uquad_control_libs base)

source_group("" FILES ${uquad_control_headers} ${uquad_control_sources})

set(uquad_control_blocks_headers
    blocks/Clock.h
    blocks/Accelerometer.h
    blocks/Gyroscope.h
    blocks/Magnetometer.h
    blocks/Barometer.h
    blocks/UQuadSensors.h
    blocks/PoseEstimation.h
    blocks/MotorsThrust.h
    blocks/QuadMotorsThrust.h
    blocks/QuadMotorsThrustSimple.h
    blocks/AttitudeControl.h
    blocks/AttitudeControlSimple.h
    blocks/PositionControl.h
    blocks/PositionControlSimple.h
    blocks/ManualControl.h
    blocks/ManualControlSimple.h
)

set(uquad_control_blocks_sources
    blocks/Clock.cpp
    blocks/Accelerometer.cpp
    blocks/Gyroscope.cpp
    blocks/Magnetometer.cpp
    blocks/Barometer.cpp
    blocks/UQuadSensors.cpp
    blocks/PoseEstimation.cpp
    blocks/MotorsThrust.cpp
    blocks/QuadMotorsThrust.cpp
    blocks/QuadMotorsThrustSimple.cpp
    blocks/AttitudeControl.cpp
    blocks/AttitudeControlSimple.cpp
    blocks/PositionControl.cpp
    blocks/PositionControlSimple.cpp
    blocks/ManualControl.cpp
    blocks/ManualControlSimple.cpp

)

set(uquad_control_blocks_libs)

source_group("blocks" FILES ${uquad_control_blocks_headers} ${uquad_control_blocks_sources})


set(UQUAD_CONTROL_BLOCKS_PE_EKF "YES" CACHE BOOL "Build ekf pose estimation control")
if(UQUAD_CONTROL_BLOCKS_PE_EKF)
	set(uquad_control_blocks_pe_ekf_headers
		blocks/pe/ekf/estimator_utilities.h
		blocks/pe/ekf/estimator_22states.h
	    blocks/pe/ekf/PoseEstimation.h
	)

	set(uquad_control_blocks_pe_ekf_sources
		blocks/pe/ekf/estimator_utilities.cpp
		blocks/pe/ekf/estimator_22states.cpp
		blocks/pe/ekf/PoseEstimation.cpp
	)

	set(uquad_control_blocks_pe_ekf_libs)

	source_group("blocks\\pe\\ekf" FILES ${uquad_control_blocks_pe_ekf_headers} ${uquad_control_blocks_pe_ekf_sources})

    set(uquad_control_blocks_headers ${uquad_control_blocks_headers} ${uquad_control_blocks_pe_ekf_headers})
    set(uquad_control_blocks_sources ${uquad_control_blocks_sources} ${uquad_control_blocks_pe_ekf_sources})
    set(uquad_control_blocks_libs ${uquad_control_blocks_libs} ${uquad_control_blocks_pe_ekf_libs})
    add_definitions(-DUQUAD_CONTROL_BLOCKS_PE_EKF)
endif()

set(UQUAD_CONTROL_BLOCKS_PE_EKF2 "YES" CACHE BOOL "Build ekf2 pose estimation")
if(UQUAD_CONTROL_BLOCKS_PE_EKF2)
	set(uquad_control_blocks_pe_ekf2_headers
	    blocks/pe/ekf2/ecl/EKF/common.h
        blocks/pe/ekf2/ecl/EKF/ekf.h
        blocks/pe/ekf2/ecl/EKF/estimator_interface.h
        blocks/pe/ekf2/ecl/EKF/RingBuffer.h
        blocks/pe/ekf2/ecl/EKF/mathlib.cpp
        blocks/pe/ekf2/ecl/EKF/geo.cpp
        blocks/pe/ekf2/PoseEstimation.h
	)

	set(uquad_control_blocks_pe_ekf2_sources
        blocks/pe/ekf2/ecl/EKF/airspeed_fusion.cpp
        blocks/pe/ekf2/ecl/EKF/control.cpp
        blocks/pe/ekf2/ecl/EKF/covariance.cpp
        blocks/pe/ekf2/ecl/EKF/ekf_helper.cpp
        blocks/pe/ekf2/ecl/EKF/ekf.cpp
        blocks/pe/ekf2/ecl/EKF/estimator_interface.cpp
        blocks/pe/ekf2/ecl/EKF/gps_checks.cpp
        blocks/pe/ekf2/ecl/EKF/mag_fusion.cpp
        blocks/pe/ekf2/ecl/EKF/optflow_fusion.cpp
        blocks/pe/ekf2/ecl/EKF/terrain_estimator.cpp
        blocks/pe/ekf2/ecl/EKF/vel_pos_fusion.cpp
        blocks/pe/ekf2/ecl/EKF/mathlib.h
        blocks/pe/ekf2/ecl/EKF/geo.h
		blocks/pe/ekf2/PoseEstimation.cpp
	)

	set(uquad_control_blocks_pe_ekf2_libs)

	source_group("blocks\\pe\\ekf2" FILES ${uquad_control_blocks_pe_ekf2_headers} ${uquad_control_blocks_pe_ekf2_sources})

	set(uquad_control_pose_estimation_ekf2_matrix_headers
		blocks/pe/ekf2/ecl/matrix/matrix/math.hpp
	    blocks/pe/ekf2/ecl/matrix/matrix/helper_functions.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Matrix.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/SquareMatrix.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Vector.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Vector2.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Vector3.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Euler.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Dcm.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Scalar.hpp
		blocks/pe/ekf2/ecl/matrix/matrix/Quaternion.hpp
	)

	set(uquad_control_blocks_pe_ekf2_matrix_sources
	)

	set(uquad_control_blocks_pe_ekf2_matrix_libs)

	source_group("blocks\\pe\\ekf2\\matrix" FILES ${uquad_control_blocks_pe_ekf2_matrix_headers} ${uquad_control_blocks_pe_ekf2_matrix_sources})

    set(uquad_control_blocks_headers ${uquad_control_blocks_headers} ${uquad_control_blocks_pe_ekf2_headers} ${uquad_control_blocks_pe_ekf2_matrix_headers})
    set(uquad_control_blocks_sources ${uquad_control_blocks_sources} ${uquad_control_blocks_pe_ekf2_sources} ${uquad_control_blocks_pe_ekf2_matrix_sources})
    set(uquad_control_blocks_libs ${uquad_control_blocks_libs} ${uquad_control_blocks_pe_ekf2_libs} ${uquad_control_blocks_pe_ekf2_matrix_libs})
    add_definitions(-DUQUAD_CONTROL_BLOCKS_PE_EKF2)
    add_definitions(-DPOSIX_SHARED)
    include_directories(blocks/pe/ekf2/ecl)
endif()

set(UQUAD_CONTROL_BLOCKS_PE_IKF "YES" CACHE BOOL "Build ikf pose estimation")
if(UQUAD_CONTROL_BLOCKS_PE_IKF)
	set(uquad_control_blocks_pe_ikf_headers
		blocks/pe/ikf/build.h
		blocks/pe/ikf/types.h
		blocks/pe/ikf/approximations.h
		blocks/pe/ikf/orientation.h
		blocks/pe/ikf/matrix.h
		blocks/pe/ikf/magnetic.h
#		ikf/mqx_tasks.h
		blocks/pe/ikf/fusion.h
	    blocks/pe/ikf/PoseEstimation.h
	)

	set(uquad_control_blocks_pe_ikf_sources
		blocks/pe/ikf/approximations.cpp
		blocks/pe/ikf/orientation.cpp
		blocks/pe/ikf/matrix.cpp
		blocks/pe/ikf/magnetic.cpp
#		ikf/mqx_tasks.cpp
		blocks/pe/ikf/fusion.cpp
		blocks/pe/ikf/PoseEstimation.cpp
	)

	set(uquad_control_blocks_pe_ikf_libs)

	source_group("blocks\\pe\\ikf" FILES ${uquad_control_blocks_pe_ikf_headers} ${uquad_control_blocks_pe_ikf_sources})

    set(uquad_control_blocks_headers ${uquad_control_blocks_headers} ${uquad_control_blocks_pe_ikf_headers})
    set(uquad_control_blocks_sources ${uquad_control_blocks_sources} ${uquad_control_blocks_pe_ikf_sources})
    set(uquad_control_blocks_libs ${uquad_control_blocks_libs} ${uquad_control_blocks_pe_ikf_libs})
    add_definitions(-DUQUAD_CONTROL_BLOCKS_PE_IKF)
endif()

set(UQUAD_CONTROL_BLOCKS_PE_MADGWICK "YES" CACHE BOOL "Build madgwick pose estimation")
if(UQUAD_CONTROL_BLOCKS_PE_MADGWICK)
	set(uquad_control_blocks_pe_madgwick_headers
        blocks/pe/madgwick/imu_filter.h
        blocks/pe/madgwick/PoseEstimation.h
	)

	set(uquad_control_blocks_pe_madgwick_sources
        blocks/pe/madgwick/imu_filter.cpp
		blocks/pe/madgwick/PoseEstimation.cpp
	)

	set(uquad_control_blocks_pe_madgwick_libs)

	source_group("blocks\\pe\\madgwick" FILES ${uquad_control_blocks_pe_madgwick_headers} ${uquad_control_blocks_pe_madgwick_sources})

    set(uquad_control_blocks_headers ${uquad_control_blocks_headers} ${uquad_control_blocks_pe_madgwick_headers})
    set(uquad_control_blocks_sources ${uquad_control_blocks_sources} ${uquad_control_blocks_pe_madgwick_sources})
    set(uquad_control_blocks_libs ${uquad_control_blocks_libs} ${uquad_control_blocks_pe_madgwick_libs})
    add_definitions(-DUQUAD_CONTROL_BLOCKS_PE_MADGWICK)
endif()

add_library(control
	STATIC
	${uquad_control_headers} ${uquad_control_blocks_headers}
    ${uquad_control_sources} ${uquad_control_blocks_sources}
)

target_link_libraries(control
	${uquad_control_libs}
	${uquad_control_blocks_libs}
)

uquad_enable_precompiled_headers(control Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
