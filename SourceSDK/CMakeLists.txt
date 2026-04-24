

add_library(vinterface STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/interface.cpp
)
target_include_directories(vinterface PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${CMAKE_CURRENT_SOURCE_DIR}/public/tier0
        ${CMAKE_CURRENT_SOURCE_DIR}/public/tier1
)
target_link_libraries(vinterface PUBLIC platform_config)

add_library(tier0 STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/assert_dialog.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/commandline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/cpu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/cpumonitoring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/cpu_usage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/dbg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/dynfunction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/fasttimer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/mem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/mem_helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/memdbg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/memstd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/memvalidate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/minidump.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/pch_tier0.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/tier0/PMELib.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/progressbar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/security.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/systeminformation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/stacktools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/thread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/threadtools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/tier0_strtools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/tslist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier0/vprof.cpp
)
if( WIN32 )
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/platform.cpp)
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/vcrmode.cpp)
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/pme_posix.cpp)
    #target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/etwprof.cpp)
elseif( POSIX )
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/cpu_posix.cpp)
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/platform_posix.cpp)
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/pme_posix.cpp)
    target_sources(tier0 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tier0/vcrmode_posix.cpp)
endif()

target_include_directories(tier0 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${CMAKE_CURRENT_SOURCE_DIR}/public/tier0
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
target_link_libraries(tier0 PUBLIC platform_config video_config)
if( WIN32 )
    target_link_libraries(tier0 PUBLIC ws2_32)
endif()
target_link_libraries(tier0 PUBLIC 3rdparty-sdl2)

add_library(tier1 STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/bitbuf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/byteswap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/characterset.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/checksum_crc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/checksum_md5.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/convar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/datamanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/diff.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/generichash.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/KeyValues.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/mempool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/memstack.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/NetAdr.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/processor_detect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/rangecheckedvar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/stringpool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/strtools.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/strtools_unicode.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/tier1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/utlbuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/utlstring.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/utlsymbol.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tier1/ilocalize.cpp
        )
if(LINUX)
    target_sources(tier1 PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/tier1/qsort_s.cpp
            )
endif()
target_include_directories(tier1 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ${CMAKE_CURRENT_SOURCE_DIR}/public/tier1
        )
target_link_libraries(tier1 PUBLIC platform_config)
if(ANDROID)
    target_link_libraries(tier1 PRIVATE android) # iconv
endif()

add_library(mathlib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/color_conversion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/halton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/lightdesc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/mathlib_base.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/powsse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/sparse_convolution_noise.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/sseconst.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/sse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/ssenoise.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/anorms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/bumpvects.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/IceKey.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/imagequant.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/polyhedron.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/quantize.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/randsse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/spherical.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/simdvectormatrix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/vector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/vmatrix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib/almostequal.cpp
        )
if(NOT MSVC)
    target_compile_options(mathlib PRIVATE -Wno-c++11-narrowing)
endif()
target_include_directories(mathlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/public/tier0
    ${CMAKE_CURRENT_SOURCE_DIR}/public/mathlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../public
)
target_link_libraries(mathlib PUBLIC platform_config)

target_link_libraries(tier1 PUBLIC vinterface tier0 mathlib)

add_library(vstdlib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/coroutine.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/cvar.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/jobthread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/KeyValuesSystem.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/osversion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/random.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/vcover.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/public/tier0/memoverride.cpp
        )
if(WIN32)
    target_sources(vstdlib PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/vstdlib/processutils.cpp
            )
endif()
target_link_libraries(vstdlib PRIVATE tier0 tier1)
target_compile_definitions(vstdlib PRIVATE -DVSTDLIB_DLL_EXPORT=1)
if(OSX)
    # fix for Gestalt
    target_link_libraries(vstdlib PUBLIC "-framework CoreServices")
endif()

add_library(SourceSDK INTERFACE)
target_include_directories(SourceSDK INTERFACE ./ ./public)
