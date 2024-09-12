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

if(DEFINED ENV{KCONFIG_CONFIG})
    set(DOTCONFIG $ENV{KCONFIG_CONFIG})
else()
    set(DOTCONFIG .config)
endif()
import_dotconfig(${DOTCONFIG})
add_config_header(config ${DOTCONFIG})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/inc)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-Os)
    add_compile_options(-fno-strict-aliasing)
    add_compile_options(-Wall -Werror)
    if(NOT DOS)
        add_compile_options(-Wpedantic)
    endif()
endif()

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
        add_link_options(-nostdlib)

        set(ANDREA_LIB ${CMAKE_CURRENT_SOURCE_DIR}/ext/andrea/lib)
        include_directories(ext/andrea/include)
        link_directories(${ANDREA_LIB})
        add_compile_definitions(CONFIG_ANDREA)
        set(DRIVERS 
            $<TARGET_FILE:gfxega>
            $<TARGET_FILE:sndmpu>
            $<TARGET_FILE:sndopl>)
    endif()
endif()

if(WIN32)
    add_compile_definitions(WINVER=0x0500)
    set(CONFIG_SDL2 1)
endif()

if(LINUX)
    add_compile_options(-g)
    set(CONFIG_SDL2 1)
endif()

if(CONFIG_SDL2)
    add_compile_definitions(CONFIG_SDL2)
endif()

include_directories(BEFORE inc)

add_subdirectory(src)
add_dependencies(lavender config)

add_custom_command(
    OUTPUT data.zip
    COMMAND
        zip -0 -r -j data.zip
        ${CMAKE_CURRENT_SOURCE_DIR}/data/*
        ${DRIVERS}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/*)

add_custom_command(
    OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND cmake -E cat $<TARGET_FILE:lavender> data.zip > sshow${CMAKE_EXECUTABLE_SUFFIX}
    DEPENDS lavender data.zip)

add_custom_target(bundle DEPENDS sshow${CMAKE_EXECUTABLE_SUFFIX})

install(TARGETS lavender DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sshow${CMAKE_EXECUTABLE_SUFFIX} DESTINATION .)
