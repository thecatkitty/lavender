include_directories(inc)
add_compile_options(-O0 -g)
add_compile_options(-Wno-pointer-arith)

add_test_executable(pal-dos-cache)
target_sources(test-pal-dos-cache PRIVATE p-cache.cpp)
target_sources(test-pal-dos-cache PRIVATE mock/fmem.cpp)
target_sources(test-pal-dos-cache PRIVATE ../p-cache.c)

add_test_executable(pal-dos-cache-xms)
target_compile_definitions(test-pal-dos-cache-xms PRIVATE CONFIG_XMS)
target_sources(test-pal-dos-cache-xms PRIVATE p-cache.cpp)
target_sources(test-pal-dos-cache-xms PRIVATE mock/xmem.cpp)
target_sources(test-pal-dos-cache-xms PRIVATE ../p-cache.c)
