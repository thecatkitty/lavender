target_link_libraries(lavender -li86)
target_link_libraries(lavender -lnstdio)
target_link_options(lavender PRIVATE -no-pie -Wl,-Map=lavender.map)

if(CONFIG_ANDREA)
    target_link_options(lavender PRIVATE -T ${ANDREA_LIB}/andrea-host.ld)
    add_custom_target(
        lavender.exe.a
        COMMAND python3
            ${ANDREA_LIB}/../libman/libman.py
            lavender.exe
            -o lavender.exe.a
        BYPRODUCTS lavender.exe.a
        DEPENDS lavender)
endif()

if(CONFIG_COMPACT)
    # FIXME: W/A for https://codeberg.org/tkchia/newlib-ia16/commit/189732ae9ff01ffaf07e0cd042150c0001f0e9f1
    # Explicit selection of a linker script shouldn't be required
    target_link_options(lavender PRIVATE -Tdos-com.ld)
    target_link_options(lavender PRIVATE -Tbss=${MAX_BUNDLE_SIZE})
endif()
