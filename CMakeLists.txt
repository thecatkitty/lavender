cmake_minimum_required(VERSION 3.24)
enable_testing()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(Lavender C CXX RC)
else()
    project(Lavender C CXX ASM)
endif()
include(cmake/extensions.cmake)

# Toolchain policy
set(CMAKE_RC_FLAGS "-c 65001")

cmake_path(GET CMAKE_C_COMPILER FILENAME COMPILER_NAME)
if(MSVC)
    include(cmake/toolchains/msvc.cmake)
elseif(CMAKE_C_COMPILER_ID MATCHES "OpenWatcom")
    include(cmake/toolchains/watcom.cmake)
else()
    include(cmake/toolchains/gcc.cmake)
endif()

# Platform configuration
if(DOS)
    set(KCONFIG_TARGET "dos")
elseif(UNIX AND NOT APPLE)
    set(LINUX 1)
    set(KCONFIG_TARGET "linux")
elseif(WIN32)
    set(KCONFIG_TARGET "windows")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

prepare_dotconfig()
import_dotconfig()
add_config_header(config)

# Platform policy
if(DOS)
    include(cmake/platforms/dos.cmake)
elseif(LINUX)
    include(cmake/platforms/linux.cmake)
elseif(WIN32)
    include(cmake/platforms/windows.cmake)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Supply missing headers for compatibility
supply_c99_header(inttypes)
supply_c99_header(minwindef)
supply_c99_header(sal)
supply_c99_header(stdbool)
supply_c99_header(stdint)
supply_c99_header(winnt)

if(CONFIG_ANDREA)
    set(DRIVERS 
        $<TARGET_FILE:gfxega>
        $<TARGET_FILE:sndmpu>
        $<TARGET_FILE:sndopl>)
endif()

# Include directories
include_directories(BEFORE inc)
include_directories(ext)

# Application
add_subdirectory(src)

# Asset archive
if(CONFIG_COMPACT)
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

# Bundle
add_custom_command(
    OUTPUT sshow${CMAKE_EXECUTABLE_SUFFIX}
    COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE_PATH=$<TARGET_FILE:lavender>
        -DARCHIVE_PATH=data.zip
        -DBUNDLE_PATH=sshow${CMAKE_EXECUTABLE_SUFFIX}
        -DMAX_SIZE=${MAX_BUNDLE_SIZE}
        -P ${CMAKE_SOURCE_DIR}/cmake/scripts/bundle.cmake
    DEPENDS lavender data.zip)

add_custom_target(bundle DEPENDS sshow${CMAKE_EXECUTABLE_SUFFIX})

if(NOT WATCOM)
    install(TARGETS lavender DESTINATION .)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sshow${CMAKE_EXECUTABLE_SUFFIX} DESTINATION .)
endif()
