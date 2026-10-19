add_library(fmt OBJECT)
target_link_libraries(fmt PRIVATE compat)

target_sources(fmt PRIVATE utf8.c)
target_sources(fmt PRIVATE zip.c)

target_sources(fmt PRIVATE bmp.c)

