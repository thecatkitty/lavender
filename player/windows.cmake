include(../cmake/extensions/win32.cmake)

target_link_libraries(lavender comctl32 version wininet winmm)
target_win32_version(lavender ${WINVER})

if(MSVC)
    if(MSVC_VERSION LESS 1500)
        target_link_libraries(lavender bufferoverflowU)
    endif()

    target_link_options(lavender PRIVATE /manifest:no /manifestuac:no)
else()
    target_link_options(lavender PRIVATE -mwindows -s)
    target_link_options(lavender PRIVATE -municode)

    if(COMPILER_NAME MATCHES "^i686")
        target_link_options(lavender PRIVATE ${CMAKE_SOURCE_DIR}/ext/libunicows/libunicows.a)
    endif()
endif()
