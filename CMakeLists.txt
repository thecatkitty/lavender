cmake_minimum_required(VERSION 3.24)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    project(Lavender C CXX RC)
else()
    project(Lavender C CXX ASM)
endif()

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
if(NOT DEFINED KCONFIG_TARGET)
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

    set(KCONFIG_TARGET "${KCONFIG_TARGET}"
        CACHE STRING "Kconfig platform target")
endif()

include(cmake/extensions/dotconfig.cmake)
prepare_dotconfig()
add_menuconfig_target()
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

# Include directories
include_directories(BEFORE inc)
include_directories(ext)

# Apply compatibility polyfills
add_subdirectory(compat)

# Tests
enable_testing()
include(cmake/extensions/test.cmake)

# Application
add_subdirectory(src)

# Asset archive
add_subdirectory(demo)

# Dependency checker
if(CONFIG_ARD)
    add_subdirectory(ard)
endif()
