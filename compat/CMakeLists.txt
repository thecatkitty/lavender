include(CheckIncludeFile)

add_library(compat INTERFACE)

function(supply_header name)
    set(result "HAS_${name}")
    check_include_file(${name}.h ${result})
    if(NOT ${result})
        target_include_directories(compat INTERFACE inc/${name})
    endif()
endfunction()

supply_header(inttypes)
supply_header(minwindef)
supply_header(sal)
supply_header(stdbool)
supply_header(stdint)
supply_header(winnt)
