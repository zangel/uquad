# extern/zlib/CMakeLists.txt

project(zlib C)

set(zlib_headers
    zlib.h
    zconf.h
    deflate.h
    trees.h
    zutil.h
    infblock.h
    inftrees.h
    infcodes.h
    infutil.h
    inffast.h
    inffixed.h
)

set(zlib_sources
  adler32.c
  compress.c
  crc32.c
  gzio.c
  uncompr.c
  deflate.c
  trees.c
  zutil.c
  inflate.c
  infblock.c
  inftrees.c
  infcodes.c
  infutil.c
  inffast.c
)

if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(deflate.c PROPERTIES COMPILE_FLAGS -O1)
endif()

add_library(z STATIC ${zlib_sources} ${zlib_headers})
