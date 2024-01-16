cmake_minimum_required(VERSION 3.16)

project("Lavender" C ASM)
include(cmake/extensions.cmake)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX 1)
endif ()

if(DOS)
    add_compile_options(-mcmodel=small -march=i8088 -Os)
    add_link_options(-mcmodel=small -Wl,--nmagic)
endif()

include_directories(BEFORE inc)

add_subdirectory(src)
