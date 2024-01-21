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
add_compile_options(-Wall -Werror -Wpedantic)

set(CMAKE_RC_FLAGS "-c 65001")
if(NOT DEFINED LAV_LANG)
    set(LAV_LANG ENU)
endif()

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

add_custom_command(
    OUTPUT data.zip
    COMMAND zip -0 -r -j data.zip ${CMAKE_CURRENT_SOURCE_DIR}/data/*
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/*)

add_custom_command(
    OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND cat src/lavender${CMAKE_EXECUTABLE_SUFFIX} data.zip > sshow${CMAKE_EXECUTABLE_SUFFIX}
    DEPENDS src/lavender${CMAKE_EXECUTABLE_SUFFIX} data.zip)

add_custom_target(bundle DEPENDS sshow${CMAKE_EXECUTABLE_SUFFIX})
