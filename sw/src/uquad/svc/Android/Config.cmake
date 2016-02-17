set(uquad_svc_android_headers
	Android/Config.h
)

set(uquad_svc_android_sources
)

source_group("Android" FILES ${uquad_svc_andrpoid_headers} ${uquad_svc_android_sources})

set(uquad_svc_headers
	${uquad_svc_headers}
	${uquad_svc_android_headers}
)

set(uquad_svc_sources
	${uquad_svc_sources}
	${uquad_svc_android_sources}
)