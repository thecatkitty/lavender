cmake_minimum_required(VERSION 3.16)

project("Lavender" C ASM)
include(cmake/extensions.cmake)

include_directories(BEFORE inc)

add_subdirectory(src)
