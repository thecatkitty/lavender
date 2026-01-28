add_executable(lard WIN32)
add_dependencies(lard version_text)
install(TARGETS lard DESTINATION .)

file(GLOB RESOURCES resource/*.rc)

target_sources(lard PRIVATE compat.c)
target_sources(lard PRIVATE config.c)
target_sources(lard PRIVATE deps.c)
target_sources(lard PRIVATE ui.c)
target_sources(lard PRIVATE winmain.c)
target_sources(lard PRIVATE action/_exec.c)
target_sources(lard PRIVATE action/instie.c)
target_sources(lard PRIVATE action/instredist.c)
target_sources(lard PRIVATE action/run.c)
target_sources(lard PRIVATE action/rundos.c)
target_sources(lard PRIVATE action/select.c)
target_sources(lard PRIVATE version/cpu.c)
target_sources(lard PRIVATE version/ie.c)
target_sources(lard PRIVATE version/dll.c)
target_sources(lard PRIVATE version/windows.c)
target_sources(lard PRIVATE ${RESOURCES})

if(WATCOM)
    foreach(resource ${RESOURCES})
        cmake_path(GET resource STEM resource_stem)
        add_custom_command(
            OUTPUT ${resource_stem}.res
            COMMAND rc /nologo /c 65001
                /i ${CMAKE_SOURCE_DIR}/inc
                /i ${CMAKE_BINARY_DIR}/inc
                /fo ${resource_stem}.res
                ${resource_stem}.rc
            COMMAND ${CMAKE_COMMAND} -E rename
                ${resource_stem}.res
                ${CMAKE_CURRENT_BINARY_DIR}/${resource_stem}.res
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resource
            VERBATIM)
        add_custom_target(lard-res-${resource_stem} DEPENDS ${resource_stem}.res)
        add_dependencies(lard lard-res-${resource_stem})
    endforeach()

    add_custom_command(
        TARGET lard POST_BUILD
        COMMAND wrc -q _neutral.res ${CMAKE_CURRENT_BINARY_DIR}/lard.exe
        COMMAND ResourceHacker -open ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -save ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -action add -res en-us.res ,,,
        COMMAND ResourceHacker -open ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -save ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -action add -res cs-cz.res ,,,
        COMMAND ResourceHacker -open ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -save ${CMAKE_CURRENT_BINARY_DIR}/lard.exe -action add -res pl-pl.res ,,,
        COMMENT "Linking resources"
        VERBATIM)
endif()

set(LARD_WINVER 0x030A)
math(EXPR LARD_WINVER_MAJOR "${LARD_WINVER} >> 8")
math(EXPR LARD_WINVER_MINOR "${LARD_WINVER} & 0xFF")

remove_definitions(-DWINVER=${WINVER})

target_compile_definitions(lard PRIVATE WINVER=0x0400) # 0x030A breaks MinGW's winreg.h
if(${COMPILER_NAME} MATCHES "i686")
    target_compile_options(lard PRIVATE -march=i386)
endif()

target_link_libraries(lard PRIVATE version)
if(MSVC)
    target_link_options(lard PRIVATE /subsystem:windows,${LARD_WINVER_MAJOR}.${LARD_WINVER_MINOR})
elseif(WATCOM)
    cmake_path(GET CMAKE_C_COMPILER PARENT_PATH WATCOM_PATH)
    set(WATCOM_LIBS ${WATCOM_PATH}/../libaxp/nt/)
    target_link_options(lard PRIVATE option nodefaultlibs)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}clib.lib)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}kernel32.lib)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}user32.lib)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}gdi32.lib)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}advapi32.lib)
    target_link_libraries(lard PRIVATE ${WATCOM_LIBS}version.lib)
else()
    target_compile_definitions(lard PRIVATE __USE_MINGW_ANSI_STDIO=0)
    target_link_options(lard PRIVATE -nostdlib -nodefaultlibs)
    target_link_options(lard PRIVATE -mwindows -s)
    target_link_options(lard PRIVATE -lcrtdll)
    target_link_options(lard PRIVATE -Wl,-e,WinMain)
    target_link_options(lard PRIVATE -Wl,--major-os-version,${LARD_WINVER_MAJOR})
    target_link_options(lard PRIVATE -Wl,--minor-os-version,${LARD_WINVER_MINOR})
    target_link_options(lard PRIVATE -Wl,--major-subsystem-version,${LARD_WINVER_MAJOR})
    target_link_options(lard PRIVATE -Wl,--minor-subsystem-version,${LARD_WINVER_MINOR})
    target_link_options(lard PRIVATE -Wl,-Map=lard.map)
endif()
