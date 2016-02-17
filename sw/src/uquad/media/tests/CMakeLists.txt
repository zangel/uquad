uquad_add_executable(testMedia
	test.cpp
	testVideoEncoder.cpp
    testVideoDecoder.cpp
    testWriter.cpp
    testGraph.cpp
)

target_link_libraries(testMedia media utility boost_unit_test_framework)

uquad_add_test(VideoEncoder testMedia)
uquad_add_test(VideoDecoder testMedia)
uquad_add_test(Writer testMedia)
uquad_add_test(GraphEncode testMedia)
uquad_add_test(GraphDecode testMedia)

