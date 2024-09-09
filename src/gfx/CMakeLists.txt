add_library(gfx OBJECT)

target_sources(gfx PRIVATE color.c)

if(DOS)
    target_sources(gfx PRIVATE dosddi.c)
    target_sources(gfx PRIVATE cga.c)
    target_sources(gfx PRIVATE vbe.c)
    target_sources(gfx PRIVATE font-8x8.c)

    if(NOT DOS_TARGET_COM)
        add_driver(gfxega)
        target_sources(gfxega PRIVATE ega.c)
        target_sources(gfxega PRIVATE errno.c)
    endif()
endif()

if(WIN32 OR LINUX)
    target_sources(gfx PRIVATE sdl2.c)
endif()
