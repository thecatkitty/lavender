cmake_minimum_required(VERSION 3.16)

if(WIN32)
    project(Lavender C RC)
else()
    project(Lavender C ASM)
endif()
include(cmake/extensions.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX 1)
endif ()

add_compile_options(-Os)
add_compile_options(-fno-strict-aliasing)
add_compile_options(-Wall -Werror)

if(DOS)
    add_compile_options(-march=i8088)
    add_link_options(-Wl,--nmagic)

    if(DOS_TARGET_COM)
        set(CMAKE_EXECUTABLE_SUFFIX ".com")
        add_compile_options(-mcmodel=tiny)
        add_link_options(-mcmodel=tiny)

        add_compile_definitions(ZIP_PIGGYBACK)
    else()
        add_compile_options(-mcmodel=small)
        add_link_options(-mcmodel=small)
    endif()
endif()

if(WIN32)
    add_compile_definitions(WINVER=0x0500)
endif()

include_directories(BEFORE inc)

add_subdirectory(src)
