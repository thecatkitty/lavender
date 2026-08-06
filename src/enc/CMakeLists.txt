add_library(enc OBJECT)

target_sources(enc PRIVATE enc.c encui.c)

add_subdirectory(cipher)
add_subdirectory(keyfmt)
add_subdirectory(provider)

if(EXISTS "${CMAKE_SOURCE_DIR}/ext/sharizard/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/ext/sharizard sharizard)
    target_link_libraries(enc PRIVATE sharizard)
endif()
