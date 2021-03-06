# extern/ffmpeg-2.1.3/libavutil/CMakeLists.txt

set(avutil_headers
	adler32.h
	aes.h
	attributes.h
	audio_fifo.h
	audioconvert.h
	avassert.h
	avstring.h
	avutil.h
	base64.h
	blowfish.h
	bprint.h
	bswap.h
	buffer.h
	channel_layout.h
	common.h
	cpu.h
	crc.h
	error.h
	eval.h
	fifo.h
	file.h
	frame.h
	hmac.h
	imgutils.h
	intfloat.h
	intfloat_readwrite.h
	intreadwrite.h
	lfg.h
	log.h
	mathematics.h
	md5.h
	mem.h
	murmur3.h
	dict.h
	old_pix_fmts.h
	opt.h
	parseutils.h
	pixdesc.h
	pixfmt.h
	random_seed.h
	rational.h
	ripemd.h
	samplefmt.h
	sha.h
	sha512.h
	time.h
	timecode.h
	timestamp.h
	version.h
	xtea.h
	lzo.h
)

set(avutil_sources
	adler32.c
	aes.c
	atomic.c
	audio_fifo.c
	avstring.c
	base64.c
	blowfish.c
	bprint.c
	buffer.c
	channel_layout.c
	cpu.c
	crc.c
	des.c
	dict.c
	error.c
	eval.c
	fifo.c
	file.c
	file_open.c
	float_dsp.c
	frame.c
	hash.c
	hmac.c
	imgutils.c
	intfloat_readwrite.c
	intmath.c
	lfg.c
	lls1.c
	lls2.c
	log.c
	log2_tab.c
	mathematics.c
	md5.c
	mem.c
	murmur3.c
	opt.c
	parseutils.c
	pixdesc.c
	random_seed.c
	rational.c
	rc4.c
	ripemd.c
	samplefmt.c
	sha.c
	sha512.c
	time.c
	timecode.c
	tree.c
	utils.c
	xga_font_data.c
	xtea.c
)

set(avutil_libs
)

if(UQUAD_PLATFORM_MAC)

	set(avutil_sources
		${avutil_sources}
			x86/cpu.c
			x86/float_dsp_init.c
			x86/lls_init.c
	)
	
elseif(UQUAD_PLATFORM_ANDROID)
	
	set(avutil_sources
		${avutil_sources}
		arm/cpu.c
		arm/float_dsp_init_arm.c
		arm/float_dsp_init_neon.c
		arm/float_dsp_init_vfp.c
		arm/float_dsp_neon.S
		arm/float_dsp_vfp.S
	)

endif()
	
add_library(avutil STATIC ${avutil_sources} ${avutil_headers})
target_link_libraries(avutil ${avutil_libs})
