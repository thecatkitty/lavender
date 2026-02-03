if(NOT DEFINED WINVER)
    set(WINVER 0x0400)
endif()
math(EXPR WINVER_MAJOR "${WINVER} >> 8")
math(EXPR WINVER_MINOR "${WINVER} & 0xFF")

message(STATUS "Building for Windows ${WINVER_MAJOR}.${WINVER_MINOR}")

add_compile_definitions(WINVER=${WINVER})

if(${COMPILER_NAME} MATCHES "^i[3-6]86")
    add_compile_options(-march=i486)
endif()
