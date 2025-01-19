cmake_minimum_required(VERSION 3.16)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(Lavender C CXX RC)
else()
    project(Lavender C CXX ASM)
endif()
include(cmake/extensions.cmake)

cmake_path(GET CMAKE_C_COMPILER FILENAME COMPILER_NAME)
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
include_directories(ext)

if(MSVC)
    if(MSVC_VERSION GREATER_EQUAL 1800)
        add_compile_options(/utf-8)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
        include_directories(inc/c99)
    endif()
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
        set(COM_MAX_SIZE 0xE000) # 56 KiB
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

    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
endif()

if(WIN32)
    if(NOT DEFINED WINVER)
        set(WINVER 0x0500)
    endif()
    add_compile_definitions(WINVER=${WINVER})
    add_compile_definitions(CONFIG_INTERNET)
    if(${COMPILER_NAME} MATCHES "i686")
        add_compile_options(-march=i586)
    endif()
endif()

if(LINUX)
    add_compile_definitions(CONFIG_INTERNET)
    add_compile_options(-g)
    set(CONFIG_SDL2 1)
endif()

if(CONFIG_SDL2)
    add_compile_definitions(CONFIG_SDL2)
endif()

include_directories(BEFORE inc)

add_subdirectory(src)
add_dependencies(lavender config)

if(DOS_TARGET_COM)
    add_custom_command(
        OUTPUT slides.txt
        COMMAND
            tail -n +2 ${CMAKE_CURRENT_SOURCE_DIR}/data/slides.txt > slides.txt
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/slides.txt)

    set(ASSETS
        slides.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/data/lavndr73.bmp
        ${CMAKE_CURRENT_SOURCE_DIR}/data/lavndr99.bmp)
else()
    set(ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
endif()

add_custom_command(
    OUTPUT data.zip
    COMMAND
        zip -0 -r -j data.zip
        ${ASSETS}
        ${DRIVERS}
    DEPENDS ${ASSETS})

if(DOS_TARGET_COM)
    add_custom_command(
        OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND
            cmake -E cat $<TARGET_FILE:lavender> data.zip > sshow${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND
            [ `stat -c %s sshow${CMAKE_EXECUTABLE_SUFFIX}` -lt `printf \"%d\\n\" ${COM_MAX_SIZE}` ] || (echo "COM file too large" && false)
        DEPENDS lavender data.zip)
else()
    add_custom_command(
        OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
        COMMAND
            cmake -E cat $<TARGET_FILE:lavender> data.zip > sshow${CMAKE_EXECUTABLE_SUFFIX}
        DEPENDS lavender data.zip)
endif()

add_custom_target(bundle DEPENDS sshow${CMAKE_EXECUTABLE_SUFFIX})

install(TARGETS lavender DESTINATION .)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sshow${CMAKE_EXECUTABLE_SUFFIX} DESTINATION .)
