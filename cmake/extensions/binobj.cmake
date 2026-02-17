execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE COMPILER_TRIPLE
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(COMPILER_TRIPLE MATCHES "^x86_64")
    set(OBJCOPY_FMT elf64-x86-64)
elseif(COMPILER_TRIPLE MATCHES "^(i[3-6]86|ia16)")
    set(OBJCOPY_FMT elf32-i386)
else()
    message(FATAL_ERROR "Unsupported target triple: ${COMPILER_TRIPLE}")
endif()


function(add_binary_object target source_file object_prefix)
    string(MAKE_C_IDENTIFIER ${source_file} __source_file_cname)
    add_custom_command(
        OUTPUT ${target}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_OBJCOPY}
            -I binary
            -O ${OBJCOPY_FMT}
            --rename-section .data=.rodata
            ${source_file}
            ${target}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_OBJCOPY}
            --redefine-sym _binary_${__source_file_cname}_start=${object_prefix}_start
            --redefine-sym _binary_${__source_file_cname}_end=${object_prefix}_end
            --redefine-sym _binary_${__source_file_cname}_size=${object_prefix}_size
            ${target}${CMAKE_C_OUTPUT_EXTENSION}
        MAIN_DEPENDENCY ${source_file})

    add_custom_target(
        ${target} ALL
        DEPENDS ${target}${CMAKE_C_OUTPUT_EXTENSION})

    set_property(
        TARGET ${target}
        PROPERTY PATH
        ${CMAKE_CURRENT_BINARY_DIR}/${target}${CMAKE_C_OUTPUT_EXTENSION})
endfunction()


function(target_link_binary_object target object)
    add_dependencies(${target} ${object})
    target_sources(${target} PRIVATE $<TARGET_PROPERTY:${object},PATH>)
endfunction()
