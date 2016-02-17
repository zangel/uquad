SET(UQUAD_CPU_ARCHITECTURE "x86_64" CACHE STRING "CPU architectures")
SET_PROPERTY(CACHE UQUAD_CPU_ARCHITECTURE PROPERTY STRINGS "i386;x86_64")

SET(CMAKE_OSX_ARCHITECTURES ${UQUAD_CPU_ARCHITECTURE} CACHE STRING "" FORCE)

SET(UQUAD_MAC_DEPLOYMENT_TARGET "10.10" CACHE STRING "Mac deployment target" FORCE)
SET_PROPERTY(CACHE UQUAD_MAC_DEPLOYMENT_TARGET PROPERTY STRINGS "10.7;10.8;10.9;10.10")

SET(CMAKE_OSX_DEPLOYMENT_TARGET ${UQUAD_MAC_DEPLOYMENT_TARGET}  CACHE STRING "" FORCE)

SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_INIT} -fvisibility=hidden -fvisibility-inlines-hidden")
SET(CMAKE_C_FLAGS_DEBUG "-g -O0")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -fvisibility=hidden -fvisibility-inlines-hidden -ftemplate-depth=1024 -Wno-error=c++11-narrowing")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS_INIT} -dead_strip")

MACRO(UQUAD_ADD_EXECUTABLE NAME)
	ADD_EXECUTABLE(${NAME} ${ARGN})
ENDMACRO(UQUAD_ADD_EXECUTABLE)

MACRO(UQUAD_INSTALL_EXECUTABLE NAME)
ENDMACRO(UQUAD_INSTALL_EXECUTABLE)

MACRO(UQUAD_ENABLE_PRECOMPILED_HEADERS TARGET HEADER)
	GET_FILENAME_COMPONENT(HEADER_ABSOLUTE ${HEADER} ABSOLUTE)
	SET_TARGET_PROPERTIES(${TARGET} 
		PROPERTIES
	    XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${HEADER_ABSOLUTE}"
		XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
	)
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
	
ENDMACRO(UQUAD_ADD_TEST)