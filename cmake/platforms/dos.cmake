if(CONFIG_XMS)
    set(ARCH "i286")
else()
    set(ARCH "i8088")
endif()

message(STATUS "Building for MS-DOS on ${ARCH}")

add_compile_options(-march=${ARCH})
add_compile_options(-Os)
add_link_options(-Wl,--nmagic)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

if(CONFIG_ANDREA)
    set(ANDREA_LIB ${CMAKE_CURRENT_SOURCE_DIR}/ext/andrea/lib)
    include_directories(ext/andrea/include)
    link_directories(${ANDREA_LIB})
endif()

if(CONFIG_COMPACT)
    set(CMAKE_EXECUTABLE_SUFFIX ".com")
    add_compile_options(-mcmodel=tiny)
    add_link_options(-mcmodel=tiny)
    set(MAX_BUNDLE_SIZE 0xE000) # 56 KiB
else()
    add_compile_options(-mcmodel=small)
    add_link_options(-mcmodel=small)
    add_link_options(-nostdlib)
endif()

include_directories(SYSTEM BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/ext/nicetia16-${ARCH}/include/c++/6.3.0)
link_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/ext/nicetia16-${ARCH}/lib)
