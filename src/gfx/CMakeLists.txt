add_library(gfx OBJECT)

target_sources(gfx PRIVATE color.c)

if(DOS)
    target_sources(gfx PRIVATE dosddi.c)
    target_sources(gfx PRIVATE cga.c)
    target_sources(gfx PRIVATE vbe.c)
    target_sources(gfx PRIVATE font-8x8.c)
endif()

if(WIN32 OR LINUX)
    target_sources(gfx PRIVATE sdl2.c)
endif()
