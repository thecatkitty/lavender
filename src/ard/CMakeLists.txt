include(../../cmake/extensions/win32.cmake)

add_executable(lard WIN32)
add_dependencies(lard version_text)
install(TARGETS lard DESTINATION .)

set(RESOURCE_NAMES _neutral en-us cs-cz pl-pl)
list(TRANSFORM RESOURCE_NAMES
    REPLACE "^(.+)$" "resource/\\1.rc"
    OUTPUT_VARIABLE RESOURCES)

target_sources(lard PRIVATE
    compat.c
    config.c
    deps.c
    ui.c
    winmain.c
    action/_exec.c
    action/instie.c
    action/instredist.c
    action/run.c
    action/rundos.c
    action/select.c
    version/cpu.c
    version/ie.c
    version/dll.c
    version/windows.c
    ${RESOURCES})

if(WATCOM)
    # Watcom does not support UTF-8 resource files, so we need to compile them
    # with the Microsoft resource compiler.
    set(resource_outputs)
    foreach(name IN LISTS RESOURCE_NAMES)
        set(resource "${CMAKE_CURRENT_SOURCE_DIR}/resource/${name}.rc")
        set(output "${CMAKE_CURRENT_BINARY_DIR}/${name}.res")

        add_custom_command(
            OUTPUT "${output}"
            COMMAND rc /nologo /c 65001
                /i "${CMAKE_SOURCE_DIR}/inc"
                /i "${CMAKE_BINARY_DIR}/inc"
                /fo "${output}"
                "${resource}"
            DEPENDS "${resource}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resource"
            COMMENT "Compiling resource ${name}"
            VERBATIM)

        list(APPEND resource_outputs "${output}")
    endforeach()

    # Watcom does not support linking multiple resource files, so we need to use
    # Resource Hacker to link them together.
    set(rh_commands)
    foreach(name IN LISTS RESOURCE_NAMES)
        if(NOT name STREQUAL "_neutral")
            string(APPEND rh_commands
                "-add \"${CMAKE_CURRENT_BINARY_DIR}/${name}.res\",,,\n")
        endif()
    endforeach()

    set(rh_script "${CMAKE_CURRENT_BINARY_DIR}/lard-resources-$<CONFIG>.txt")
    file(GENERATE OUTPUT "${rh_script}"
        CONTENT "[FILENAMES]
Open=\"$<TARGET_FILE:lard>\"
Save=\"$<TARGET_FILE:lard>\"
[COMMANDS]
${rh_commands}")

    add_custom_command(
        TARGET lard POST_BUILD
        COMMAND wrc -q
            "${CMAKE_CURRENT_BINARY_DIR}/_neutral.res"
            "$<TARGET_FILE:lard>"
        COMMAND ResourceHacker -script "${rh_script}"
        COMMENT "Linking resources"
        VERBATIM)

    # Ensure that the resource files are built before lard is linked.
    add_custom_target(lard-resources DEPENDS ${resource_outputs})
    add_dependencies(lard lard-resources)
    set_property(TARGET lard APPEND PROPERTY LINK_DEPENDS ${resource_outputs})
endif()

remove_definitions(-DWINVER=${WINVER})
target_compile_definitions(lard PRIVATE WINVER=0x0400) # 0x030A breaks MinGW's winreg.h
if(COMPILER_NAME MATCHES "^i686")
    target_compile_options(lard PRIVATE -march=i386)
endif()

target_link_libraries(lard PRIVATE version)
target_win32_version(lard 0x030A)

if(NOT MSVC AND NOT WATCOM)
    if(EXISTS "${PROJECT_SOURCE_DIR}/ext/bulwa-crtdll")
        target_link_options(lard PRIVATE -B "${PROJECT_SOURCE_DIR}/ext/bulwa-crtdll/lib/i386")
    endif()

    target_link_options(lard PRIVATE -s)
    target_link_options(lard PRIVATE -lcrtdll)
    target_link_options(lard PRIVATE -Wl,-Map=lard.map)
endif()
