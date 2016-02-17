uquad_add_executable(testBase
	test.cpp
	testRefCounted.cpp
	testDelegate.cpp
    testFileMappingMemory.cpp
    testImage.cpp
#    testYieldContext.cpp
    testYieldEvent.cpp
    testIOArchive.cpp
    testObjectFactory.cpp
	testGraph.cpp
)

target_link_libraries(testBase base boost_signals boost_chrono boost_unit_test_framework)

uquad_add_test(RefCounted testBase)
uquad_add_test(Delegate testBase)
uquad_add_test(FileMappingMemory testBase)
uquad_add_test(Image testBase)
#uquad_add_test(YieldContext testBase)
uquad_add_test(YieldEvent testBase)
uquad_add_test(IOArchive testBase)
uquad_add_test(ObjectFactory testBase)
uquad_add_test(Graph testBase)

