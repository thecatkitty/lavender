add_library(enc OBJECT)
add_dependencies(enc config)

target_sources(enc PRIVATE enc.c)

add_subdirectory(cipher)
add_subdirectory(keyfmt)
add_subdirectory(provider)
