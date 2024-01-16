add_library(gfx OBJECT)

target_sources(gfx PRIVATE color.c)

target_sources(gfx PRIVATE sdl2.c)
