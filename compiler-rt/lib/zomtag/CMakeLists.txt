add_compiler_rt_component(zomtag)

set(ZOMTAG_SOURCES
  zomtag.c)

include_directories(..)

set(ZOMTAG_CFLAGS)

#foreach(arch ${ZOMTAG_SUPPORTED_ARCH})
#  add_compiler_rt_runtime(clang_rt.zomtag
#    STATIC
#    ARCHS ${arch}
#    SOURCES ${ZOMTAG_SOURCES}
#            $<TARGET_OBJECTS:RTInterception.${arch}>
#            $<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
#            $<TARGET_OBJECTS:RTSanitizerCommonNoLibc.${arch}>
#    CFLAGS ${ZOMTAG_CFLAGS}
#    PARENT_TARGET zomtag)
#endforeach()

add_compiler_rt_runtime(clang_rt.zomtag
        STATIC
        ARCHS aarch64
        SOURCES ${ZOMTAG_SOURCES}
        CFLAGS ${ZOMTAG_CFLAGS}
        PARENT_TARGET zomtag)

add_sanitizer_rt_symbols(clang_rt.zomtag)
add_dependencies(compiler-rt zomtag)
