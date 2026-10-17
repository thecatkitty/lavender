include(CheckIncludeFile)

add_library(compat OBJECT)
target_include_directories(compat PUBLIC inc/pub)

function(supply_header name)
    set(result "HAS_${name}")
    check_include_file(${name}.h ${result})
    if(NOT ${result})
        target_include_directories(compat PUBLIC inc/${name})
    endif()
endfunction()

supply_header(inttypes)
supply_header(minwindef)
supply_header(sal)
supply_header(stdbool)
supply_header(stdint)
supply_header(winnt)

target_sources(compat PRIVATE
    src/bswap.c)
