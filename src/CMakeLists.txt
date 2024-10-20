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

if(CONFIG_ENCRYPTED_CONTENT)
    add_subdirectory(dlg)
    add_subdirectory(enc)
endif()
add_subdirectory(fmt)
add_subdirectory(gfx)
add_subdirectory(pal)
add_subdirectory(sld)
add_subdirectory(snd)

if(MSVC)
    target_link_options(lavender PRIVATE /manifest:no /manifestuac:no)
else()
    target_link_options(lavender PRIVATE -no-pie -Wl,-Map=lavender.map)
endif()
include_directories(lavender ${CMAKE_CURRENT_BINARY_DIR})

target_sources(lavender PRIVATE main.c rstd.c)
if(CONFIG_ENCRYPTED_CONTENT)
    target_link_libraries(lavender dlg enc)
endif()
target_link_libraries(lavender fmt gfx pal sld snd)

add_custom_target(version_text
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/../cmake/version.cmake)
if(WIN32)
    add_dependencies(lavender version_text)
    target_sources(lavender PRIVATE resource.${LAV_LANG}.rc)
else()
    add_custom_command(
        OUTPUT version.bin
        COMMAND sed -E \"s/$$/\\x0/\" version.txt > version.bin
        DEPENDS version_text)
    add_binary_object(version version.bin _binary_obj_version_txt)
    target_link_binary_object(lavender version)

    add_win32_strings(strings resource.${LAV_LANG}.rc)
    target_link_win32_strings(lavender strings)
endif()

if(DOS)
    if(NOT DOS_TARGET_COM)
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
    target_link_libraries(lavender -li86)
endif()

if(WIN32)
    if (NOT MSVC)
        target_link_options(lavender PRIVATE -mwindows -s)
        target_link_options(lavender PRIVATE -municode)
        target_link_options(lavender PRIVATE -Wl,--major-os-version,5)
        target_link_options(lavender PRIVATE -Wl,--major-subsystem-version,5)
    endif()
    target_link_libraries(lavender comctl32 msimg32 version winmm)
endif()

if(LINUX)
    target_link_libraries(lavender blkid)
    target_link_libraries(lavender ${Fontconfig_LIBRARIES})
    target_link_libraries(lavender ${SDL2_LIBRARIES} SDL2_ttf)
    target_link_libraries(lavender fluidsynth)
endif()
