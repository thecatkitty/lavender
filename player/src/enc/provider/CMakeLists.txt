target_sources(enc PRIVATE caller.c)
target_sources(enc PRIVATE diskid.c)
target_sources(enc PRIVATE prompt.c)
target_sources(enc PRIVATE split.c)

add_subdirectory(remote)
