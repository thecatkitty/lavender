include(CheckIncludeFile)


function(supply_c99_header name)
    check_include_file(${name}.h has_c99_header)
    if(NOT has_c99_header)
        include_directories(inc/c99/${name})
    endif()
endfunction()
