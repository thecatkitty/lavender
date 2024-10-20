add_library(enc OBJECT)
add_dependencies(enc config)

target_sources(enc PRIVATE enc.c)
target_sources(enc PRIVATE keyutils.c)
target_sources(enc PRIVATE des.c)
target_sources(enc PRIVATE xor.c)
