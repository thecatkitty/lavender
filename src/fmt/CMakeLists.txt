add_library(fmt OBJECT)
target_link_libraries(fmt PRIVATE compat)

target_sources(fmt PRIVATE utf8.c)
