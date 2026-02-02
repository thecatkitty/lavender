add_executable(lavender WIN32)

if(CONFIG_SDL2)
    find_package(SDL2 REQUIRED PATHS ../ext)
    if(SDL2_FOUND)
        message(STATUS "Found SDL2 - ${SDL2_DIR}")
    endif()
    include_directories(lavender ${SDL2_INCLUDE_DIR} ${SDL2_INCLUDE_DIR}/..)
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

if(NOT WIN32)
    set_property(TARGET lavender PROPERTY CXX_STANDARD 14)
    target_link_libraries(lavender -lstdc++)
endif()

include_directories(lavender ${CMAKE_CURRENT_BINARY_DIR})
target_sources(lavender PRIVATE main.c rstd.c)
target_link_libraries(lavender fmt gfx pal sld)

file(GLOB LANG_RESOURCES resource/[a-z]*.rc)
add_custom_target(version_text
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/version.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
if(WIN32)
    add_dependencies(lavender version_text)
    target_sources(lavender PRIVATE resource/_neutral.rc)
    target_sources(lavender PRIVATE ${LANG_RESOURCES})
else()
    add_custom_command(
        OUTPUT version.bin
        COMMAND sed -E \"s/$$/\\x0/\" ${CMAKE_BINARY_DIR}/version.txt > version.bin
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

include(${KCONFIG_TARGET}.cmake)

if(LINUX)
    add_subdirectory(pal/dos/test)
endif()
