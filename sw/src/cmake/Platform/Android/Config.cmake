SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build configuration")
SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -fvisibility=hidden -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -fvisibility=hidden -fvisibility-inlines-hidden -Wall")

IF(CMAKE_COMPILER_IS_CLANG)
	SET(DISABLED_COMMON_WARNINGS "-Wno-deprecated-register -Wno-unused-local-typedef -Wno-deprecated-declarations -Wno-c++11-narrowing -Wno-unused-private-field -Wno-error=c++11-narrowing -Wno-unsequenced -Wno-switch -Wno-unused-variable -Wno-pointer-sign")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DISABLED_COMMON_WARNINGS}")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DISABLED_COMMON_WARNINGS}")
ELSE()
	SET(DISABLED_COMMON_WARNINGS "-Wno-narrowing -Wno-enum-compare -Wno-deprecated-declarations -Wno-sign-compare -Wno-format -Wno-comment -Wno-unused-but-set-variable -Wno-unused-variable -Wno-uninitialized -Wno-switch")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DISABLED_COMMON_WARNINGS} -Wno-delete-non-virtual-dtor")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DISABLED_COMMON_WARNINGS} -Wno-pointer-sign")
ENDIF()

IF(ANDROID_NDK)
	IF(${ANDROID_NDK_RELEASE_VERSION} STRGREATER "r9" OR ${ANDROID_NDK_RELEASE_VERSION} STRGREATER "r10")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
	ENDIF()
ENDIF()


MACRO(UQUAD_ADD_EXECUTABLE NAME)
	ADD_EXECUTABLE(${NAME} ${ARGN})
	
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		
		ADD_CUSTOM_COMMAND(TARGET ${NAME}
			POST_BUILD
			COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/${NAME} ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.debug
			COMMAND ${CMAKE_STRIP} --strip-debug --strip-unneeded ${CMAKE_CURRENT_BINARY_DIR}/${NAME}
			COMMENT "stripping ${NAME} ..."
		)
		
		FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gdb_setup
			"file ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.debug"
		)
		
		FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/debug_${NAME}
			"#!/system/bin/sh\n"
			"killall gdbserver >/dev/null 2>&1\n"
			"killall ${NAME} >/dev/null 2>&1\n"
			"cd /data/local/tmp/${NAME}\n"
			"nohup gdbserver :5039 ./${NAME} </dev/null >/dev/null 2>&1 &\n"
			"sleep 2\n"
		)
		
		ADD_CUSTOM_TARGET(copy_to_device_${NAME}
			COMMAND adb shell 'mkdir -p /data/local/tmp/${NAME}'
			COMMAND adb push ${CMAKE_CURRENT_BINARY_DIR}/${NAME} /data/local/tmp/${NAME}/${NAME}
			COMMAND adb push ${CMAKE_CURRENT_BINARY_DIR}/debug_${NAME} /data/local/tmp/${NAME}/debug_${NAME}
			COMMAND adb shell 'chmod 755 /data/local/tmp/${NAME}/debug_${NAME}'
		)
		ADD_DEPENDENCIES(copy_to_device_${NAME} ${NAME})
	
		ADD_CUSTOM_TARGET(debug_${NAME}
			COMMAND adb shell '/data/local/tmp/${NAME}/debug_${NAME}'
		)
	
	ELSE()
		
		ADD_CUSTOM_TARGET(copy_to_device_${NAME}
			COMMAND adb shell 'mkdir -p /data/local/tmp/${NAME}'
			COMMAND adb push ${CMAKE_CURRENT_BINARY_DIR}/${NAME} /data/local/tmp/${NAME}/${NAME}
			COMMAND adb shell 'chmod 755 /data/local/tmp/${NAME}/debug_${NAME}'
		)
		ADD_DEPENDENCIES(copy_to_device_${NAME} ${NAME})
		
	ENDIF()
	
	SET(RUN_DIR "/data/local/tmp")
	ADD_CUSTOM_TARGET(run_${NAME}
		COMMAND ${UQT_COMMAND} -e "\"android.test.run(${ANDROID_SDK},${RUN_DIR},${CMAKE_CURRENT_BINARY_DIR}/${NAME},\\\"\\\",())\""
	)
	
ENDMACRO(UQUAD_ADD_EXECUTABLE)

MACRO(UQUAD_INSTALL_EXECUTABLE NAME)
ENDMACRO(UQUAD_INSTALL_EXECUTABLE)

MACRO(UQUAD_EXPORT_ALL_TARGET_FLAGS TARGET FILENAME)
	STRING(TOUPPER "CMAKE_CXX_FLAGS" EXPORT_CMAKE_CXX_FLAGS_NAME)
	STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" EXPORT_CMAKE_CXX_FLAGS_CONFIG_NAME)
	SET(EXPORT_COMPILER_FLAGS ${${EXPORT_CMAKE_CXX_FLAGS_NAME}})
	SET(EXPORT_COMPILER_FLAGS_CONFIG ${${EXPORT_CMAKE_CXX_FLAGS_CONFIG_NAME}})
	SET(EXPORT_COMPILER_FLAGS "${EXPORT_COMPILER_FLAGS}\n${EXPORT_COMPILER_FLAGS_CONFIG}\n")
	
	
	#GET_DIRECTORY_PROPERTY(EXPORT_DIRECTORY_INCLUDE_DIRECTORIES_LIST INCLUDE_DIRECTORIES)
	#FOREACH(EXPORT_DIRECTORY_INCLUDE_DIRECTORIES_DIR ${EXPORT_DIRECTORY_INCLUDE_DIRECTORIES_LIST})
	#	SET(EXPORT_DIRECTORY_INCLUDE_DIRECTORIES "${EXPORT_DIRECTORY_INCLUDE_DIRECTORIES}-I${EXPORT_DIRECTORY_INCLUDE_DIRECTORIES_DIR}\n")
	#ENDFOREACH(EXPORT_DIRECTORY_INCLUDE_DIRECTORIES_DIR)
	
	#GET_DIRECTORY_PROPERTY(EXPORT_DIRECTORY_DEFINITIONS DEFINITIONS)
	#STRING(STRIP "${EXPORT_DIRECTORY_DEFINITIONS}" EXPORT_DIRECTORY_DEFINITIONS)
	#SET(EXPORT_DIRECTORY_DEFINITIONS "${EXPORT_DIRECTORY_DEFINITIONS}\n")
		
	SET(EXPORT_TARGET_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")
	SET(EXPORT_TARGET_INCLUDE_DIRECTORIES "$<$<BOOL:${EXPORT_TARGET_INCLUDE_DIRECTORIES}>:-I$<JOIN:${EXPORT_TARGET_INCLUDE_DIRECTORIES},\n-I>\n>")
	
	SET(EXPORT_TARGET_COMPILE_DEFINITIONS "$<TARGET_PROPERTY:${TARGET},COMPILE_DEFINITIONS>")
	SET(EXPORT_TARGET_COMPILE_DEFINITIONS "$<$<BOOL:${EXPORT_TARGET_COMPILE_DEFINITIONS}>:-D$<JOIN:${EXPORT_TARGET_COMPILE_DEFINITIONS},\n-D>\n>")
	
	SET(EXPORT_TARGET_COMPILE_FLAGS "$<TARGET_PROPERTY:${TARGET},COMPILE_FLAGS>")
	SET(EXPORT_TARGET_COMPILE_FLAGS "$<$<BOOL:${EXPORT_TARGET_COMPILE_FLAGS}>:$<JOIN:${EXPORT_TARGET_COMPILE_FLAGS},\n>\n>")
	
	SET(EXPORT_TARGET_COMPILE_OPTIONS "$<TARGET_PROPERTY:${TARGET},COMPILE_OPTIONS>")
	SET(EXPORT_TARGET_COMPILE_OPTIONS "$<$<BOOL:${EXPORT_TARGET_COMPILE_OPTIONS}>:$<JOIN:${EXPORT_TARGET_COMPILE_OPTIONS},\n>\n>")
	
	FILE(GENERATE OUTPUT "${FILENAME}" CONTENT "${EXPORT_COMPILER_FLAGS}${EXPORT_TARGET_COMPILE_FLAGS}${EXPORT_TARGET_COMPILE_OPTIONS}${EXPORT_TARGET_INCLUDE_DIRECTORIES}${EXPORT_TARGET_COMPILE_DEFINITIONS}\n")
ENDMACRO(UQUAD_EXPORT_ALL_TARGET_FLAGS)

MACRO(UQUAD_ENABLE_PRECOMPILED_HEADERS TARGET HEADER)
	SET(PCH_TARGET ${TARGET}_pch)
	GET_FILENAME_COMPONENT(HEADER_ABSOLUTE ${HEADER} ABSOLUTE)
	GET_FILENAME_COMPONENT(HEADER_NAME ${HEADER} NAME)
	
	SET(PCH_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_pch)
	SET(PCH_BINARY_HEADER ${PCH_BINARY_DIR}/prec.h)
	SET(PCH_BINARY_OUTPUT ${PCH_BINARY_DIR}/prec.h.gch)
	
	MAKE_DIRECTORY(${PCH_BINARY_DIR})
	
	FILE(WRITE ${PCH_BINARY_HEADER} "#include \"${HEADER_ABSOLUTE}\"\n")
	
	SET(PCH_COMPILE_FLAGS_FILE ${PCH_BINARY_DIR}/compile_flags.rsp)
	UQUAD_EXPORT_ALL_TARGET_FLAGS(${TARGET} ${PCH_COMPILE_FLAGS_FILE})
	SET(PCH_COMPILE_FLAGS "@${PCH_COMPILE_FLAGS_FILE}")
	
	ADD_CUSTOM_COMMAND(
	      OUTPUT ${PCH_BINARY_OUTPUT}
	      COMMAND ${CMAKE_CXX_COMPILER} ${PCH_COMPILE_FLAGS} -x c++-header -o ${PCH_BINARY_OUTPUT} ${PCH_BINARY_HEADER}
	      DEPENDS ${PCH_BINARY_HEADER} ${PCH_COMPILE_FLAGS_FILE}
		  IMPLICIT_DEPENDS CXX ${PCH_BINARY_HEADER}
	      COMMENT "Precompiling headers for target '${TARGET}'"
	)
	
	ADD_CUSTOM_TARGET(${PCH_TARGET} DEPENDS ${PCH_BINARY_OUTPUT})
	ADD_DEPENDENCIES(${TARGET} ${PCH_TARGET})
	SET_TARGET_PROPERTIES(${TARGET} PROPERTIES COMPILE_FLAGS "-include ${PCH_BINARY_HEADER} -Winvalid-pch")

ENDMACRO(UQUAD_ENABLE_PRECOMPILED_HEADERS)

MACRO(UQUAD_ADD_TEST NAME TEST_APP)
	SET(OPTION_ARGS)
	SET(ONE_VALUE_ARGS)
	SET(MULTI_VALUE_ARGS ARGS)
	CMAKE_PARSE_ARGUMENTS(TEST_APP_ARG "${OPTION_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})
	
	UNSET(TEST_RUN_ARGS)
	FOREACH(TEST_APP_ARG_ARGS_ARG ${TEST_APP_ARG_ARGS})
		IF(TEST_RUN_ARGS)
			SET(TEST_RUN_ARGS "${TEST_RUN_ARGS},")
		ENDIF()
		SET(TEST_RUN_ARGS "${TEST_RUN_ARGS}\"${TEST_APP_ARG_ARGS_ARG}\"")
	ENDFOREACH()
	
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	
		FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/debug_${TEST_APP}_${NAME}
			"#!/system/bin/sh\n"
			"killall gdbserver >/dev/null 2>&1\n"
			"killall ${TEST_APP} >/dev/null 2>&1\n"
			"cd /data/local/tmp/${TEST_APP}\n"
			"nohup gdbserver :5039 ./${TEST_APP} --run_test=${NAME} </dev/null >/dev/null 2>&1 &\n"
			"sleep 2\n"
		)
	
		ADD_CUSTOM_TARGET(copy_to_device_${TEST_APP}_${NAME}
			COMMAND adb push ${CMAKE_CURRENT_BINARY_DIR}/debug_${TEST_APP}_${NAME} /data/local/tmp/${TEST_APP}/debug_${TEST_APP}_${NAME}
			COMMAND adb shell 'chmod 755 /data/local/tmp/${TEST_APP}/debug_${TEST_APP}_${NAME}'
		)
		ADD_DEPENDENCIES(copy_to_device_${TEST_APP}_${NAME} copy_to_device_${TEST_APP})
		
	
		ADD_CUSTOM_TARGET(debug_${TEST_APP}_${NAME}
			COMMAND adb shell '/data/local/tmp/${TEST_APP}/debug_${TEST_APP}_${NAME}'
		)
	ENDIF()
	
	SET(TEST_RUN_DIR "/data/local/tmp")
	ADD_TEST(${NAME} COMMAND ${UQT_COMMAND} -e "android.test.run(${ANDROID_SDK},${TEST_RUN_DIR},${CMAKE_CURRENT_BINARY_DIR}/${TEST_APP},--run_test=${NAME},(${TEST_RUN_ARGS}))")
	
	ADD_CUSTOM_TARGET(run_${TEST_APP}_${NAME}
		COMMAND ${UQT_COMMAND} -e "\"android.test.run(${ANDROID_SDK},${TEST_RUN_DIR},${CMAKE_CURRENT_BINARY_DIR}/${TEST_APP},--run_test=${NAME},(${TEST_RUN_ARGS}))\""
	)
	
ENDMACRO(UQUAD_ADD_TEST)