# extern/ffmpeg-2.1.3/CMakeLists.txt

project(cvd)
include(CheckFunctionExists)

if(UQUAD_PLATFORM_MAC)
	set(CVD_INTERNAL_NEED_TR1 0)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.mac ${PROJECT_BINARY_DIR}/cvd/config.h @ONLY)
elseif(UQUAD_PLATFORM_ANDROID)
	set(CVD_INTERNAL_NEED_TR1 0)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.android ${PROJECT_BINARY_DIR}/cvd/config.h @ONLY)
elseif(UQUAD_PLATFORM_IOS)
    set(CVD_INTERNAL_NEED_TR1 0)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.ios ${PROJECT_BINARY_DIR}/cvd/config.h @ONLY)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TooN/include)

set(cvd_headers
	cvd/abs.h
	cvd/argb.h
	cvd/bgrx.h
	cvd/bresenham.h
	cvd/brezenham.h
	cvd/byte.h
	cvd/camera.h
	cvd/colourspace.h
	cvd/colourspace_convert.h
	cvd/colourspace_frame.h
	cvd/colourspacebuffer.h
	cvd/colourspaces.h
	cvd/connected_components.h
	cvd/convolution.h
	cvd/cpu_hacks.h
	cvd/cvd_image.h
	cvd/cvd_timer.h
	cvd/deinterlacebuffer.h
	cvd/deinterlacebuffer.h
	cvd/diskbuffer2.h
	cvd/diskbuffer2_frame.h
	cvd/documentation.h
	cvd/draw.h
	cvd/eventobject.h
	cvd/exceptions.h
	cvd/fast_corner.h
	cvd/gl_helpers.h
	cvd/glwindow.h
	cvd/haar.h
	cvd/harris_corner.h
	cvd/helpers.h
	cvd/image.h
	cvd/image_convert.h
	cvd/image_convert_fwd.h
	cvd/image_interpolate.h
	cvd/image_io.h
	cvd/image_ref.h
	cvd/integral_image.h
	cvd/interpolate.h
	cvd/integral_image.h
	cvd/irls.h
	cvd/la.h
	cvd/localvideobuffer.h
	cvd/localvideoframe.h
	cvd/message_queue.h
	cvd/morphology.h
	cvd/nonmax_suppression.h
	cvd/random.h
	cvd/readaheadvideobuffer.h
	cvd/rgb.h
	cvd/rgb8.h
	cvd/rgba.h
	cvd/ringbuffer.h
	cvd/runnable.h
	cvd/runnable_batch.h
	cvd/serverpushjpegbuffer.h
	cvd/serverpushjpegframe.h
	cvd/synchronized.h
	cvd/tensor_voting.h
	cvd/thread.h
	cvd/timeddiskbuffer.h
	cvd/timer.h
	cvd/utility.h
	cvd/vector_image_ref.h
	cvd/videobuffer.h
	cvd/videobufferflags.h
	cvd/videobufferwithdata.h
	cvd/videodisplay.h
	cvd/videofilebuffer.h
	cvd/videofilebuffer_frame.h
	cvd/videoframe.h
	cvd/videosource.h
	cvd/vision.h
	cvd/vision_exceptions.h
	cvd/yc.h
)
source_group("cvd" FILES ${cvd_headers})

set(cvd_internal_headers
	cvd/internal/aligned_mem.h
	cvd/internal/builtin_components.h
	cvd/internal/convert_pixel_types.h
	cvd/internal/gl_types.h
	cvd/internal/image_ref_implementation.hh
	cvd/internal/is_pod.h
	cvd/internal/load_and_save.h
	cvd/internal/name_builtin_types.h
	cvd/internal/name_CVD_rgb_types.h
	cvd/internal/pixel_operations.h
	cvd/internal/pixel_traits.h
	cvd/internal/rgb_components.h
	cvd/internal/scalar_convert.h
)
source_group("cvd\\internal" FILES ${cvd_internal_headers})

set(cvd_internal_io_headers
	cvd/internal/io/bmp.h
	cvd/internal/io/cvdimage.h
	cvd/internal/io/fits.h
	cvd/internal/io/jpeg.h
	cvd/internal/io/parameter.h
	cvd/internal/io/png.h
	cvd/internal/io/pnm_grok.h
	cvd/internal/io/save_postscript.h
	cvd/internal/io/text.h
	cvd/internal/io/tiff.h
)
source_group("cvd\\internal\\io" FILES ${cvd_internal_io_headers})

set(cvd_Linux_headers
	cvd/Linux/dvbuffer.h
	cvd/Linux/dvbuffer3.h
	cvd/Linux/dvframe.h
	cvd/Linux/v4l1buffer.h
	cvd/Linux/v4l1frame.h
	cvd/Linux/v4l2buffer.h
	cvd/Linux/v4l2frame.h
	cvd/Linux/v4lbuffer.h
	cvd/Linux/v4lcontrol.h
)
source_group("cvd\\Linux" FILES ${cvd_Linux_headers})

set(cvd_OSX_headers
	cvd/OSX/qtbuffer.h
	cvd/OSX/qtframe.h

)
source_group("cvd\\OSX" FILES ${cvd_OSX_headers})


set(cvd_sources
	cvd_src/bayer.cxx
	cvd_src/brezenham.cc
	cvd_src/colourspace_convert.cxx
	cvd_src/connected_components.cc
	cvd_src/convolution.cc
	cvd_src/corner_10.h
	cvd_src/corner_12.h
	cvd_src/corner_9.h
	cvd_src/cvd_timer.cc
	cvd_src/deinterlacebuffer.cc
	cvd_src/diskbuffer2.cc
	cvd_src/draw.cc
	cvd_src/draw_toon.cc
	cvd_src/eventobject.cpp
	cvd_src/exceptions.cc
	cvd_src/fast_corner.cxx
	cvd_src/fast_corner_9_nonmax.cxx
	cvd_src/image_io.cc
	cvd_src/mono.h
	cvd_src/morphology.cc
	cvd_src/nonmax_suppression.cxx
	cvd_src/sans.h
	cvd_src/serif.h
	cvd_src/slower_corner_10.cxx
	cvd_src/slower_corner_11.cxx
	cvd_src/slower_corner_12.cxx
	cvd_src/slower_corner_7.cxx
	cvd_src/slower_corner_8.cxx
	cvd_src/slower_corner_9.cxx
	cvd_src/synchronized.cpp
	cvd_src/tensor_voting.cc
	cvd_src/thread.cpp
	cvd_src/timeddiskbuffer.cc
	cvd_src/utility_helpers.h
	cvd_src/videosource.cpp
	cvd_src/videosource_nodvbuffer.cc
	cvd_src/videosource_novideofilebuffer.cc
	cvd_src/yuv411_to_stuff.cxx
	cvd_src/yuv420.cpp
	cvd_src/yuv422.cpp
	cvd_src/yuv422.h
)
if(NOT UQUAD_PLATFORM_ANDROID)
	list(APPEND cvd_sources "cvd_src/globlist.cxx")
endif()
source_group("cvd_src" FILES ${cvd_sources})

set(cvd_image_io_sources
	cvd_src/image_io/bmp.cxx
	cvd_src/image_io/bmp_read.cc
	cvd_src/image_io/bmp_write.cc
	cvd_src/image_io/cvdimage.cxx
	cvd_src/image_io/fits.cc
	cvd_src/image_io/fitswrite.cc
	cvd_src/image_io/pnm_grok.cxx
	cvd_src/image_io/save_postscript.cxx
	cvd_src/image_io/text.cxx
	cvd_src/image_io/text_write.cc
)
source_group("cvd_src\\image_io" FILES ${cvd_image_io_sources})

set(cvd_fast_sources
	cvd_src/fast/fast_10_detect.cxx
	cvd_src/fast/fast_10_score.cxx
	cvd_src/fast/fast_11_detect.cxx
	cvd_src/fast/fast_11_score.cxx
	cvd_src/fast/fast_12_detect.cxx
	cvd_src/fast/fast_12_score.cxx
	cvd_src/fast/fast_7_detect.cxx
	cvd_src/fast/fast_7_score.cxx
	cvd_src/fast/fast_8_detect.cxx
	cvd_src/fast/fast_8_score.cxx
	cvd_src/fast/fast_9_detect.cxx
	cvd_src/fast/fast_9_score.cxx
	cvd_src/fast/prototypes.h
)
source_group("cvd_src\\fast" FILES ${cvd_fast_sources})

set(cvd_Linux_sources
	cvd_src/Linux/videosource_nov4l1buffer.cc
	cvd_src/Linux/videosource_nov4lbuffer.cc
)
source_group("cvd_src\\Linux" FILES ${cvd_Linux_sources})

set(cvd_noarch_sources
	cvd_src/noarch/convert_rgb_to_y.cc
	cvd_src/noarch/convolve_gaussian.cc
	cvd_src/noarch/default_memalign.cpp
	cvd_src/noarch/gradient.cc
	cvd_src/noarch/median_3x3.cc
	cvd_src/noarch/posix_memalign.cpp
	cvd_src/noarch/two_thirds_sample.cc
	cvd_src/noarch/utility_byte_differences.cc
	cvd_src/noarch/utility_double_int.cc
	cvd_src/noarch/utility_float.cc
	cvd_src/noarch/yuv422_wrapper.cc
    cvd_src/noarch/half_sample.cc
)
source_group("cvd_src\\noarch" FILES ${cvd_noarch_sources})

set(cvd_nothread_sources
	cvd_src/nothread/runnable_batch.cc

)
source_group("cvd_src\\nothread" FILES ${cvd_nothread_sources})

set(cvd_OSX_sources
	cvd_src/OSX/videosource_noqtbuffer.cc
)
if(UQUAD_PLATFORM_MAC OR UQUAD_PLATFORM_IOS)
	list(APPEND cvd_OSX_sources "cvd_src/apple/get_time_of_day_ns.cc")
elseif(UQUAD_PLATFORM_ANDROID)
	list(APPEND cvd_OSX_sources "cvd_src/posix/get_time_of_day_ns.cc")
endif()


source_group("cvd_src\\OSX" FILES ${cvd_OSX_sources})

set(cvd_posix_sources
)
source_group("cvd_src\\posix" FILES ${cvd_posix_sources})


set(cvd_thread_sources
	cvd_src/thread/runnable_batch.cc

)
source_group("cvd_src\\thread" FILES ${cvd_thread_sources})

set(cvd_libs lapack
)

add_library(cvd STATIC
	${cvd_headers} ${cvd_internal_headers} ${cvd_internal_io_headers} ${cvd_Linux_headers} ${cvd_OSX_headers}
	${cvd_sources} ${cvd_image_io_sources} ${cvd_fast_sources} ${cvd_Linux_sources} ${cvd_noarch_sources} ${cvd_nothread_sources} ${cvd_OSX_sources} ${cvd_posix_sources} ${cvd_thread_sources}
)
target_link_libraries(cvd ${cvd_libs})
