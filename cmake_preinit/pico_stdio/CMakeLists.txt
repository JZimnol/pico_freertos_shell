# create an INTERFACE library named target, and define LIB_TARGET=1 (upper case) as a compile option
# and make it dependent on a pre-existing corresponding _headers library
# optional arg NOFLAG will skip the LIB_TARGET definition
function(pico_add_impl_library target)
    add_library(${target} INTERFACE)
    string(TOUPPER ${target} TARGET_UPPER)
    if (${ARGC} GREATER 1)
        if (NOT "${ARGV1}" STREQUAL "NOFLAG")
            message(FATAL_ERROR "Unknown parameter ${ARGV1}")
        endif()
    else()
        target_compile_definitions(${target} INTERFACE LIB_${TARGET_UPPER}=1)
    endif()
    target_link_libraries(${target} INTERFACE ${target}_headers)
endfunction()

# create an INTERFACE library named target along with associated header, and define LIB_TARGET=1 (upper case) as a compile option
# optional arg NOFLAG will skip the LIB_TARGET definition
function(pico_add_library target)
    add_library(${target}_headers INTERFACE)
    pico_add_impl_library(${target} ${ARGN})
endfunction()

# add a link option to wrap the given function name; i.e. -Wl:wrap=FUNCNAME for gcc
function(pico_wrap_function TARGET FUNCNAME)
    target_link_options(${TARGET} INTERFACE "LINKER:--wrap=${FUNCNAME}")
endfunction()

# takes dependencies after the target
function(pico_mirrored_target_link_libraries TARGET SCOPE)
    if (${ARGC} LESS 3)
        message(FATAL_ERROR "expected a target, scope and at least one dependency")
    endif()
    if (NOT TARGET ${TARGET}_headers)
        message(FATAL_ERROR "${TARGET} does not have headers")
    endif()
    # library should depend on its own header
    target_link_libraries(${TARGET} ${SCOPE} ${TARGET}_headers)
    foreach(DEPENDENCY IN LISTS ARGN)
        if (DEPENDENCY MATCHES ".*_headers")
            message(FATAL_ERROR "should not use headers with pico_mirrored_target_link_libraries")
        endif()
        # note, it would be nice to only add the dependency if it exists, but we do
        # not necessarily add libraries in reverse dependency order, so we do this
        # unconditionally, so this function should only be passed dependencies that
        # have headers, or link will fail with a missing library -lfoo_headers
        target_link_libraries(${TARGET}_headers ${SCOPE} ${DEPENDENCY}_headers)
        target_link_libraries(${TARGET} ${SCOPE} ${DEPENDENCY})
    endforeach()
endfunction()

if (NOT TARGET pico_stdio)
    pico_add_library(pico_stdio)

    target_include_directories(pico_stdio_headers INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

    target_sources(pico_stdio INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/stdio.c
    )

    pico_wrap_function(pico_stdio printf)
    pico_wrap_function(pico_stdio vprintf)
    pico_wrap_function(pico_stdio puts)
    pico_wrap_function(pico_stdio putchar)
    pico_wrap_function(pico_stdio getchar)

    if (TARGET pico_printf)
        pico_mirrored_target_link_libraries(pico_stdio INTERFACE pico_printf)
    endif()

    target_link_libraries(pico_stdio INTERFACE pico_freertos_shell_lib)
endif()
