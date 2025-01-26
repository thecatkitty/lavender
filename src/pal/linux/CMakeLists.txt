add_subdirectory(../_sdl2 sdl2)
target_sources(pal PRIVATE ../shared/p-strings.c)
target_sources(pal PRIVATE ../shared/p-version.c)

target_sources(pal PRIVATE p-alert.c)
target_sources(pal PRIVATE p-id.c)
target_sources(pal PRIVATE p-init.c)
target_sources(pal PRIVATE p-shell.c)
target_sources(pal PRIVATE p-state.c)
target_sources(pal PRIVATE p-time.c)
target_sources(pal PRIVATE p-volume.c)

target_sources(pal PRIVATE l-info.c)
