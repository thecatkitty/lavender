add_library(sld OBJECT)
add_dependencies(sld config)

target_sources(sld PRIVATE context.c)
target_sources(sld PRIVATE loader.c)
target_sources(sld PRIVATE runner.c)
target_sources(sld PRIVATE actarea.c)
target_sources(sld PRIVATE bitmap.c)
target_sources(sld PRIVATE call.c)
target_sources(sld PRIVATE jump.c)
target_sources(sld PRIVATE text.c)
target_sources(sld PRIVATE play.c)
target_sources(sld PRIVATE query.c)
target_sources(sld PRIVATE shape.c)
