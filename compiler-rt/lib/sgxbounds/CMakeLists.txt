include_directories(..)

set(SGXBOUNDS_RTL_SOURCES
			runtime.c
			#wrappers.c	
		)

set(SGXBOUNDS_RTL_HEADERS
			common.h
			uthash.h
		)

set(SGXBOUNDS_COMMON_CFLAGS ${SANITIZER_COMMON_CFLAGS})
add_compiler_rt_component(sgxbounds)

foreach(arch ${SGXBOUNDS_SUPPORTED_ARCH})
	set(SGXBOUNDS_CFLAGS ${SGXBOUNDS_COMMON_CFLAGS})
	append_list_if(COMPILER_RT_HAS_FPIE_FLAG -fPIE SGXBOUNDS_CFLAGS)
	add_compiler_rt_runtime(clang_rt.sgxbounds
		STATIC
		ARCHS ${arch}
		SOURCES ${SGXBOUNDS_RTL_SOURCES}
		$<TARGET_OBJECTS:RTInterception.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommonLibc.${arch}>
		CFLAGS ${SGXBOUNDS_CFLAGS}
		PARENT_TARGET sgxbounds)
	add_sanitizer_rt_symbols(clang_rt.sgxbounds
		ARCHS ${arch}
	)

	add_dependencies(sgxbounds
		clang_rt.sgxbounds-${arch}-symbols)
endforeach()
