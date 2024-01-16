add_subdirectory(crg)
add_subdirectory(dlg)
add_subdirectory(fmt)
add_subdirectory(gfx)
add_subdirectory(pal)
add_subdirectory(sld)
add_subdirectory(snd)

add_executable(lavender)
target_compile_options(lavender PRIVATE -no-pie)
target_link_options(lavender PRIVATE -no-pie -Wl,-Map=lavender.map)
include_directories(lavender PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_sources(lavender PRIVATE main.c rstd.c)
target_link_libraries(lavender crg dlg fmt gfx pal sld snd)

add_custom_target(version_text
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/../cmake/version.cmake)
add_custom_command(
    OUTPUT version.bin
    COMMAND sed -E \"s/$$/\\x0/\" version.txt > version.bin
    DEPENDS version_text)
add_binary_object(version version.bin _binary_obj_version_txt)
target_link_binary_object(lavender version)

add_win32_strings(strings resource.ENU.rc)
target_link_win32_strings(lavender strings)

find_package(Fontconfig REQUIRED)
target_link_libraries(lavender ${Fontconfig_LIBRARIES})

find_package(SDL2 REQUIRED)
target_link_libraries(lavender ${SDL2_LIBRARIES} SDL2_ttf)

target_link_libraries(lavender fluidsynth)
