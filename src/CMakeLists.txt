add_executable(lavender WIN32)

if(CONFIG_SDL2)
    find_package(SDL2 REQUIRED PATHS ../ext)
    if(SDL2_FOUND)
        message("Found SDL2 - ${SDL2_DIR}")
    endif()
    include_directories(lavender ${SDL2_INCLUDE_DIR} ${SDL2_INCLUDE_DIR}/..)
endif()

if(LINUX)
    find_package(Fontconfig REQUIRED)
endif()

add_subdirectory(fmt)
add_subdirectory(gfx)
add_subdirectory(pal)
add_subdirectory(sld)

if(${LAV_ARD})
    add_subdirectory(ard)
endif()

if(CONFIG_ENCRYPTED_CONTENT)
    add_subdirectory(enc)
    target_link_libraries(lavender enc)
endif()

if(CONFIG_INTERNET)
    add_subdirectory(net)
    target_link_libraries(lavender net)
endif()

if(CONFIG_SOUND)
    add_subdirectory(snd)
    target_link_libraries(lavender snd)
endif()

if(DOS)
    target_include_directories(lavender SYSTEM BEFORE PRIVATE ../ext/nicetia16-${ARCH}/include/c++/6.3.0)
    target_link_directories(lavender BEFORE PRIVATE ../ext/nicetia16-${ARCH}/lib)
endif()

if(DOS AND CONFIG_COMPACT)
    # FIXME: W/A for https://codeberg.org/tkchia/newlib-ia16/commit/189732ae9ff01ffaf07e0cd042150c0001f0e9f1
    # Explicit selection of a linker script shouldn't be required
    target_link_options(lavender PRIVATE -Tdos-com.ld)
    target_link_options(lavender PRIVATE -Tbss=${MAX_BUNDLE_SIZE})
endif()

if(NOT WIN32)
    set_property(TARGET lavender PROPERTY CXX_STANDARD 14)
    target_link_libraries(lavender -lstdc++)
endif()

if(MSVC)
    target_link_options(lavender PRIVATE /manifest:no /manifestuac:no)
else()
    target_link_options(lavender PRIVATE -no-pie -Wl,-Map=lavender.map)
endif()
include_directories(lavender ${CMAKE_CURRENT_BINARY_DIR})

target_sources(lavender PRIVATE main.c rstd.c)
target_link_libraries(lavender fmt gfx pal sld)

file(GLOB LANG_RESOURCES resource/[a-z]*.rc)
add_custom_target(version_text
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/../cmake/scripts/version.cmake)
if(WIN32)
    add_dependencies(lavender version_text)
    target_sources(lavender PRIVATE resource/_neutral.rc)
    target_sources(lavender PRIVATE ${LANG_RESOURCES})
else()
    add_custom_command(
        OUTPUT version.bin
        COMMAND sed -E \"s/$$/\\x0/\" version.txt > version.bin
        DEPENDS version_text)
    add_binary_object(version version.bin _binary_obj_version_txt)
    target_link_binary_object(lavender version)

    if(CONFIG_COMPACT)
        if(NOT DEFINED LAV_LANG)
            set(LAV_LANG en-us)
        endif()
        target_link_win32_strings(lavender resource/${LAV_LANG}.rc "")
    else()
        foreach(lang_resource ${LANG_RESOURCES})
            get_filename_component(lang_name ${lang_resource} NAME_WLE)
            string(REPLACE "-" "_" suffix ${lang_name})
            string(TOUPPER ${suffix} suffix)
            target_link_win32_strings(lavender resource/${lang_name}.rc ${suffix})
        endforeach()
    endif()
endif()

if(DOS)
    if(CONFIG_ANDREA)
        target_link_options(lavender PRIVATE -T ${ANDREA_LIB}/andrea-host.ld)
        add_custom_target(
            lavender.exe.a
            COMMAND python3
                ${ANDREA_LIB}/../libman/libman.py
                lavender.exe
                -o lavender.exe.a
            BYPRODUCTS lavender.exe.a
            DEPENDS lavender)
    endif()
    target_link_libraries(lavender -lnstdio)
    target_link_libraries(lavender -li86)
endif()

if(WIN32)
    math(EXPR WINVER_MAJOR "${WINVER} >> 8")
    math(EXPR WINVER_MINOR "${WINVER} & 0xFF")

    if (NOT MSVC)
        target_link_options(lavender PRIVATE -mwindows -s)
        target_link_options(lavender PRIVATE -municode)
        target_link_options(lavender PRIVATE -Wl,--major-os-version,${WINVER_MAJOR})
        target_link_options(lavender PRIVATE -Wl,--minor-os-version,${WINVER_MINOR})
        target_link_options(lavender PRIVATE -Wl,--major-subsystem-version,${WINVER_MAJOR})
        target_link_options(lavender PRIVATE -Wl,--minor-subsystem-version,${WINVER_MINOR})

        cmake_path(GET CMAKE_C_COMPILER FILENAME compiler_name)
        if(${compiler_name} MATCHES "^i[3-6]86")
            target_link_options(lavender PRIVATE ${CMAKE_SOURCE_DIR}/ext/libunicows/libunicows.a)
        endif()
    else()
        target_link_options(lavender PRIVATE /subsystem:windows,${WINVER_MAJOR}.${WINVER_MINOR})
    endif()
    target_link_libraries(lavender comctl32 version wininet winmm)

    if(MSVC)
        if(MSVC_VERSION LESS 1500)
            target_link_libraries(lavender bufferoverflowU)
        endif()
    endif()
endif()

if(LINUX)
    target_link_libraries(lavender blkid)
    target_link_libraries(lavender curl)
    target_link_libraries(lavender ${Fontconfig_LIBRARIES})
    target_link_libraries(lavender ${SDL2_LIBRARIES} SDL2_ttf)
    target_link_libraries(lavender fluidsynth)
endif()

if(LINUX)
    add_subdirectory(pal/dos/test)
endif()
