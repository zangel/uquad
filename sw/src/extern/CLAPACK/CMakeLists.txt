project(clapack)

add_definitions(-DNO_BLAS_WRAP)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shift-op-parentheses -Wno-logical-op-parentheses -Wno-bitwise-op-parentheses -Wno-format")


file(GLOB F77_sources ${PROJECT_SOURCE_DIR}/F2CLIBS/libF77/*.c)
list(REMOVE_ITEM F77_sources
    "${PROJECT_SOURCE_DIR}/F2CLIBS/libF77/pow_qq.c"
    "${PROJECT_SOURCE_DIR}/F2CLIBS/libF77/qbitbits.c"
    "${PROJECT_SOURCE_DIR}/F2CLIBS/libF77/qbitshft.c"
)
list(REMOVE_ITEM F77_sources
    "${PROJECT_SOURCE_DIR}/F2CLIBS/libF77/main.c"
)
add_library(F77 STATIC ${F77_sources})


file(GLOB blas_sources ${PROJECT_SOURCE_DIR}/BLAS/SRC/*.c)
add_library(blas STATIC ${blas_sources})


file(GLOB lapack_sources ${PROJECT_SOURCE_DIR}/SRC/*.c)
add_library(lapack STATIC ${lapack_sources})
target_link_libraries(lapack blas F77)