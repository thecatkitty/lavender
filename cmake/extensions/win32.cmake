function(target_win32_version target version)
    math(EXPR major "${version} >> 8")
    math(EXPR minor "${version} & 0xFF")

    if(MSVC)
        target_link_options(${target} PRIVATE
            "/subsystem:windows,${major}.${minor}")
    elseif(WATCOM)
        target_link_options(${target} PRIVATE
            runtime "windows=${major}.${minor}")
    else()
        target_link_options(${target} PRIVATE
            "LINKER:--major-os-version,${major}"
            "LINKER:--minor-os-version,${minor}"
            "LINKER:--major-subsystem-version,${major}"
            "LINKER:--minor-subsystem-version,${minor}")
    endif()
endfunction()
