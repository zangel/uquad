
set(uquad_comm_headers
	Config.h
    Log.h
    Message.h
    Channel.h
    RemoteControl.h
    RemoteControlServer.h
    RemoteControlClient.h
    CameraServer.h
    CameraClient.h

)

set(uquad_comm_sources
	Config.cpp
    Message.cpp
    Channel.cpp
    RemoteControl.cpp
    RemoteControlServer.cpp
    RemoteControlClient.cpp
    CameraServer.cpp
    CameraClient.cpp
)

set(uquad_comm_msg_headers
    msg/ArmMotors.h
)
	
set(uquad_comm_msg_sources
    msg/ArmMotors.cpp
)
	
source_group("msg" FILES ${uquad_comm_msg_headers} ${uquad_comm_msg_sources})

set(uquad_comm_libs media boost_serialization)

source_group("" FILES ${uquad_comm_headers} ${uquad_comm_sources})

add_library(comm
	STATIC
	${uquad_comm_headers} ${uquad_comm_msg_headers}
    ${uquad_comm_sources} ${uquad_comm_msg_sources}
)

target_link_libraries(comm ${uquad_comm_libs})

uquad_enable_precompiled_headers(comm Config.h)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()