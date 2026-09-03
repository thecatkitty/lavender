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

if(CONFIG_ARD)
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

include_directories(lavender ${CMAKE_CURRENT_BINARY_DIR})
target_sources(lavender PRIVATE main.c rstd.c)
target_link_libraries(lavender fmt gfx pal sld)

add_custom_target(version_text
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/scripts/version.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
if(WIN32)
    add_dependencies(lavender version_text)
else()
    add_custom_command(
        OUTPUT version.bin
        COMMAND sed -E \"s/$$/\\x0/\" ${CMAKE_BINARY_DIR}/version.txt > version.bin
        DEPENDS version_text)
    add_binary_object(version version.bin _binary_obj_version_txt)
    target_link_binary_object(lavender version)
endif()

add_subdirectory(resource)

include(${KCONFIG_TARGET}.cmake)

if(CONFIG_TESTS)
    add_subdirectory(pal/dos/test)
endif()
