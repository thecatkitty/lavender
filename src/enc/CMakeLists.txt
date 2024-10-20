add_library(enc OBJECT)
add_dependencies(enc config)

target_sources(enc PRIVATE enc.c)
target_sources(enc PRIVATE keyutils.c)

add_subdirectory(cipher)
