# extern/ffmpeg-2.1.3/libavcodec/CMakeLists.txt

set(avavcodec_headers
	avcodec.h
	avfft.h
	dxva2.h
	old_codec_ids.h
	vaapi.h
	vda.h
	vdpau.h
	version.h
	xvmc.h
)

set(avcodec_libs
	avutil
)

if(UQUAD_PLATFORM_MAC)
	
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/x86)
	set(avcodec_sources
		aandcttab.c
		ac3tab.c
		allcodecs.c
		audioconvert.c
		avpacket.c
		avpicture.c
		bitstream.c
		bitstream_filter.c
		cabac.c
		codec_desc.c
		dsputil.c
		error_resilience.c
		faandct.c
		faanidct.c
		flvdec.c
		flvenc.c
		fmtconvert.c
		frame_thread_encoder.c
		golomb.c
		h263.c
		h263_parser.c
		h263dec.c
		h264.c
		h264_cabac.c
		h264_cavlc.c
		h264_direct.c
		h264_loopfilter.c
		h264_ps.c
		h264_refs.c
		h264_sei.c
		h264chroma.c
		h264dsp.c
		h264idct.c
		h264pred.c
		h264qpel.c
		hpeldsp.c
		imgconvert.c
		intelh263dec.c
		ituh263dec.c
		
		ituh263enc.c
		jfdctfst.c
		jfdctint.c
		jrevdct.c
		libx264.c
		mathtables.c
		motion_est.c
		mpeg12data.c
		mpeg4audio.c
		mpeg4video.c
		mpeg4video_parser.c
		mpeg4videodec.c
		mpeg4videoenc.c
		mpegaudiodata.c
		mpegvideo.c
		mpegvideo_enc.c
		mpegvideo_motion.c
		options.c
		parser.c
		pthread.c
		ratecontrol.c
		raw.c
		rawdec.c
		resample.c
		resample2.c
        simple_idct.c
		utils.c
		videodsp.c
	)
	
	set(avcodec_sources
		${avcodec_sources}
			x86/constants.c
			x86/dsputil_init.c
			x86/dsputil_mmx.c
			x86/dsputil_x86.c
			x86/dsputilenc_mmx.c
			x86/fdct.c
			x86/fmtconvert_init.c
			x86/fpel_mmx.c
			x86/h264_intrapred_init.c
			x86/h264_qpel.c
			x86/h264chroma_init.c
			x86/h264dsp_init.c
			x86/hpeldsp_init.c
			x86/hpeldsp_mmx.c
			x86/idct_mmx_xvid.c
			x86/idct_sse2_xvid.c
			x86/motion_est.c
			x86/mpegvideo.c
			x86/mpegvideoenc.c
			x86/rnd_mmx.c
			x86/simple_idct.c
			x86/videodsp_init.c
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../x264)
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../x264)
    set(avcodec_libs ${avcodec_libs} x264 iconv)
	
	
		
elseif(UQUAD_PLATFORM_ANDROID)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/arm)
	
	set(avcodec_sources
		allcodecs.c
		audioconvert.c
		avpacket.c
		avpicture.c
		bitstream.c
		bitstream_filter.c
		codec_desc.c
		fmtconvert.c
		imgconvert.c
		mathtables.c
		mpeg4audio.c
		mpegaudiodata.c
		options.c
		parser.c
		pthread.c
		raw.c
		rawdec.c
		resample.c
		resample2.c
		utils.c
	)
	
	set(avcodec_sources
		${avcodec_sources}
			arm/fmtconvert_init_arm.c
			arm/fmtconvert_neon.S
			arm/fmtconvert_vfp.S
			arm/fmtconvert_vfp_armv6.S
	)
	
endif()

add_library(avcodec STATIC ${avcodec_sources} ${avavcodec_headers})
target_link_libraries(avcodec ${avcodec_libs})
