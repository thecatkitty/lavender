add_library(fmt OBJECT)

target_sources(fmt PRIVATE utf8.c)
target_sources(fmt PRIVATE zip.c)

target_sources(fmt PRIVATE bmp.c)

target_sources(fmt PRIVATE iff.c)
target_sources(fmt PRIVATE midi.c)
