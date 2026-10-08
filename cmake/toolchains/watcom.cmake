set(WATCOM 1)

cmake_policy(SET CMP0136 NEW) # Watcom runtime library flags are selected by an abstraction
set(CMAKE_WATCOM_RUNTIME_LIBRARY "MultiThreaded")

if(COMPILER_NAME MATCHES "^wclaxp")
    set(CMAKE_C_CREATE_WIN32_EXE "system ntaxp_win")
endif()
