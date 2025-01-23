add_library(pal OBJECT)
add_dependencies(pal config)

target_sources(pal PRIVATE strings.c)
target_sources(pal PRIVATE ziparch.c)

if(DOS)
    target_sources(pal PRIVATE dos/dos.c)
    target_sources(pal PRIVATE dos/pit.S)
    target_sources(pal PRIVATE dos/pp.c)
endif()

if(CONFIG_SDL2)
    target_sources(pal PRIVATE sdl2.c)
endif()

if(WIN32)
    target_sources(pal PRIVATE windows/windows.c)
endif()

if(LINUX)
    target_sources(pal PRIVATE log.c)
    target_sources(pal PRIVATE linux.c)
endif()
