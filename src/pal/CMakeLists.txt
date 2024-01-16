add_library(pal OBJECT)

target_sources(pal PRIVATE ziparch.c)

target_sources(pal PRIVATE sdl2arch.c)

target_sources(pal PRIVATE log.c)
target_sources(pal PRIVATE linux.c)
