set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

if(MSVC_VERSION GREATER_EQUAL 1800)
    add_compile_options(/utf-8)
endif()
