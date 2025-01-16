include(CheckTypeSize)
check_type_size("void *" POINTER_SIZE)

cmake_path(GET CMAKE_C_COMPILER FILENAME compiler_name)
if(${compiler_name} MATCHES "-")
    string(REGEX REPLACE "-[a-z]+$" "-" compiler_prefix ${compiler_name})
endif()

if((${compiler_prefix} MATCHES "x86_64") OR (("${compiler_prefix}" STREQUAL "") AND ("${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "x86_64")))
    set(OBJCOPY_FMT elf64-x86-64)
else()
    set(OBJCOPY_FMT elf32-i386)
endif()

function(target_link_win32_strings target source_file)
    if(LINUX)
        set(win32_strings_args "-D__linux__")
    endif()

    add_custom_command(
        OUTPUT ${source_file}.c
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/strconv.py
            ${CMAKE_CURRENT_SOURCE_DIR}/${source_file}
            ${source_file}.c
            ${CMAKE_C_COMPILER}
            --
            -DSTRINGS_ONLY
            ${win32_strings_args}
            -I${CMAKE_SOURCE_DIR}/inc/
            -I${CMAKE_BINARY_DIR}/inc/
        MAIN_DEPENDENCY ${source_file})

    target_sources(${target} PRIVATE ${source_file}.c)
    set_source_files_properties(${source_file}.c PROPERTIES COMPILE_FLAGS -Wno-pedantic)
endfunction()


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


function(import_dotconfig dotconfig_file)
    file(STRINGS ${dotconfig_file} DOTCONFIG_FILE
        ENCODING "UTF-8")

    foreach(LINE ${DOTCONFIG_FILE})
        if("${LINE}" MATCHES "^(CONFIG_[^=]+)=([yn]|.+$)")
            set(KCONFIG_VARIABLE_NAME "${CMAKE_MATCH_1}")
            set(KCONFIG_VARIABLE_VALUE "${CMAKE_MATCH_2}")
        elseif("${LINE}" MATCHES "^# (${prefix}[^ ]+) is not set")
            set(KCONFIG_VARIABLE_NAME "${CMAKE_MATCH_1}")
            set(KCONFIG_VARIABLE_VALUE "n")
        else()
            continue()
        endif()

        if("${KCONFIG_VARIABLE_VALUE}" STREQUAL "n")
            unset("${KCONFIG_VARIABLE_NAME}" PARENT_SCOPE)
        else()
            if("${KCONFIG_VARIABLE_VALUE}" MATCHES "^\"(.*)\"$")
                set(KCONFIG_VARIABLE_VALUE ${CMAKE_MATCH_1})
            endif()
            set("${KCONFIG_VARIABLE_NAME}" "${KCONFIG_VARIABLE_VALUE}" PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

function(add_config_header target source_file)
    file(MAKE_DIRECTORY inc/generated)

    add_custom_command(
        OUTPUT inc/generated/${target}.h
        COMMAND ${CMAKE_COMMAND}
            -Din=${CMAKE_CURRENT_SOURCE_DIR}/${source_file}
            -Dout=inc/generated/${target}.h
            -P ${CMAKE_CURRENT_LIST_DIR}/cmake/dotconfig.cmake
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${source_file})

        add_custom_target(
            ${target} ALL
            DEPENDS inc/generated/${target}.h)
endfunction()


function(add_driver target)
    add_executable(${target})
    set_target_properties(${target} PROPERTIES SUFFIX ".sys")
    target_compile_definitions(${target} PRIVATE LOADABLE)
    target_link_options(${target} PRIVATE -T ${ANDREA_LIB}/andrea-module.ld)
    target_link_options(${target} PRIVATE -Wl,-Map=${target}.map)
    target_link_libraries(${target} ${CMAKE_BINARY_DIR}/src/lavender.exe.a)
    target_link_libraries(${target} -lc -li86)
    add_dependencies(${target} lavender.exe.a)
endfunction()
