function(target_link_win32_strings target source_file suffix)
    if("${suffix}" STREQUAL "")
        set(suffix_arg1 "")
        set(suffix_arg2 "")
        set(subtarget ${target}-strings)
    else()
        set(suffix_arg1 "--suffix")
        set(suffix_arg2 "_${suffix}")
        set(subtarget ${target}-strings-${suffix})
    endif()

    get_filename_component(destination_file
        "${CMAKE_CURRENT_BINARY_DIR}/${source_file}.c"
        ABSOLUTE)
    add_custom_command(
        OUTPUT ${destination_file}
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/strconv.py
            ${suffix_arg1} ${suffix_arg2}
            ${CMAKE_CURRENT_SOURCE_DIR}/${source_file}
            ${destination_file}
            ${CMAKE_C_COMPILER}
            --
            -DSTRINGS_ONLY
            ${win32_strings_args}
            -I${CMAKE_SOURCE_DIR}/inc/
            -I${CMAKE_BINARY_DIR}/inc/
        MAIN_DEPENDENCY ${source_file})

    add_custom_target(${subtarget} ALL DEPENDS ${destination_file})
    add_dependencies(${target} ${subtarget})
    target_sources(${target} PRIVATE ${destination_file})
endfunction()


file(GLOB LANG_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} [a-z]*.rc)
if(WIN32)
    target_sources(lavender PRIVATE _neutral.rc)
    target_sources(lavender PRIVATE ${LANG_RESOURCES})
elseif(CONFIG_COMPACT)
    if(NOT DEFINED LAV_LANG)
        set(LAV_LANG en-us)
    endif()
    target_link_win32_strings(lavender ${LAV_LANG}.rc "")
else()
    foreach(lang_resource ${LANG_RESOURCES})
        get_filename_component(lang_name ${lang_resource} NAME_WLE)
        string(REPLACE "-" "_" suffix ${lang_name})
        string(TOUPPER ${suffix} suffix)
        target_link_win32_strings(lavender ${lang_resource} ${suffix})
    endforeach()
endif()
