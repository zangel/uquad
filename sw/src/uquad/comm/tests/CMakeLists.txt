uquad_add_executable(testComm
	test.cpp
    testCameraServer.cpp
    testMsg.cpp
    testChannel.cpp
    testRemoteControl.cpp
)

target_link_libraries(testComm comm boost_unit_test_framework)

uquad_add_test(CameraServer testComm)
uquad_add_test(Msg testComm)
uquad_add_test(Channel testComm)
uquad_add_test(RemoteControl testComm)
