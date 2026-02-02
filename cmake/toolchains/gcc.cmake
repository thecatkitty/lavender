add_compile_options(-Wall -Werror)
if(NOT DOS)
    add_compile_options(-Wpedantic)
endif()

add_compile_options(-fno-strict-aliasing)

add_compile_options("$<$<CONFIG:RELEASE>:-Os>")

add_compile_options("$<$<CONFIG:DEBUG>:-g>")
