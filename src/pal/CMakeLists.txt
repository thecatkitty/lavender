add_library(pal OBJECT)

target_sources(pal PRIVATE ziparch.c)

if(DOS)
    target_sources(pal PRIVATE dospc.S)
    target_sources(pal PRIVATE dospc.c)
endif()

if(LINUX)
    target_sources(pal PRIVATE sdl2arch.c)
    target_sources(pal PRIVATE log.c)
    target_sources(pal PRIVATE linux.c)
endif()
