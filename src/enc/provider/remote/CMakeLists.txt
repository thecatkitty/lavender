target_sources(enc PRIVATE remote.c)

target_sources(enc PRIVATE ccode.c)
target_sources(enc PRIVATE qr.c)
target_sources(enc PRIVATE rcode.c)

add_subdirectory(qr)
