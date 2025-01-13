target_sources(enc PRIVATE remote.c)

target_sources(enc PRIVATE ccode.c)
target_sources(enc PRIVATE qr.c)
target_sources(enc PRIVATE rcode.c)

if (WIN32 OR LINUX)
    target_sources(enc PRIVATE inet.c)
    add_subdirectory(json)
endif()

add_subdirectory(qr)
