add_library(pal OBJECT)
add_dependencies(pal config)

target_sources(pal PRIVATE strings.c)
target_sources(pal PRIVATE ziparch.c)

if(DOS)
    add_subdirectory(dos)
endif()

if(CONFIG_SDL2)
    target_sources(pal PRIVATE sdl2.c)
endif()

if(WIN32)
    add_subdirectory(windows)
endif()

if(LINUX)
    add_subdirectory(linux)
    target_sources(pal PRIVATE log.c)
endif()
