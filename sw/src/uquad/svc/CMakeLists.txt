set(uquad_svc_headers
	Config.h
)

set(uquad_svc_sources
	Config.cpp
)

source_group("" FILES ${uquad_svc_headers} ${uquad_svc_sources})

add_library(svc
	STATIC
	${uquad_svc_headers}
	${uquad_svc_sources}
)