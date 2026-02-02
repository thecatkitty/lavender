find_package(Fontconfig REQUIRED)

target_link_libraries(lavender blkid)
target_link_libraries(lavender curl)
target_link_libraries(lavender ${Fontconfig_LIBRARIES})
target_link_libraries(lavender ${SDL2_LIBRARIES} SDL2_ttf)
target_link_libraries(lavender fluidsynth)
