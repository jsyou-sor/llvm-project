add_compiler_rt_component(zomtag)

set(ZOMTAG_SOURCES
		zomtag.c)

include_directories(..)

set(ZOMTAG_COMMON_CFLAGS ${SANITIZER_COMMON_CFLAGS})

foreach(arch ${ZOMTAG_SUPPORTED_ARCH})
	set(ZOMTAG_CFLAGS ${ZOMTAG_COMMON_CFLAGS})
	append_list_if(COMPILER_RT_HAS_FPIE_FLAGS -fPIE ZOMTAG_CFLAGS)
	add_compiler_rt_runtime(clang_rt.zomtag
		STATIC
		ARCHS ${arch}
		SOURCES ${ZOMTAG_SOURCES}
		$<TARGET_OBJECTS:RTInterception.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommonLibc.${arch}>
		CFLAGS ${ZOMTAG_CFLAGS}
		PARENT_TARGET zomtag)
	add_sanitizer_rt_symbols(clang_rt.zomtag
		ARCHS ${arch}
	)
	add_dependencies(zomtag
		clang_rt.zomtag-${arch}-symbols)
endforeach()

