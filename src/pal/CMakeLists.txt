add_library(pal OBJECT)

target_sources(pal PRIVATE ziparch.c)

if(DOS)
    target_sources(pal PRIVATE dospc.S)
    target_sources(pal PRIVATE dospc.c)
endif()

if(CONFIG_SDL2)
    target_sources(pal PRIVATE sdl2arch.c)
endif()

if(WIN32)
    target_sources(pal PRIVATE windows.c)
endif()

if(LINUX)
    target_sources(pal PRIVATE log.c)
    target_sources(pal PRIVATE linux.c)

    # FIXME: W/A for https://sourceware.org/bugzilla/show_bug.cgi?id=30719
    if(POINTER_SIZE EQUAL 8)
        target_sources(pal PRIVATE linux.S)
    endif()
endif()
