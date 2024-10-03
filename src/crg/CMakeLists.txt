add_library(crg OBJECT)
add_dependencies(crg config)

target_sources(crg PRIVATE crg.c)
target_sources(crg PRIVATE keyutils.c)
target_sources(crg PRIVATE des.c)
target_sources(crg PRIVATE xor.c)
