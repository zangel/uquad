set(uquad_svc_mac_headers
	Mac/Config.h
)

set(uquad_svc_mac_sources
)

source_group("Mac" FILES ${uquad_svc_mac_headers} ${uquad_svc_mac_sources})

set(uquad_svc_headers
	${uquad_svc_headers}
	${uquad_svc_mac_headers}
)

set(uquad_svc_sources
	${uquad_svc_sources}
	${uquad_svc_mac_sources}
)