add_library(net OBJECT)
add_dependencies(net config)

if(WIN32)
    target_sources(net PRIVATE wininet.c)
endif()

if(LINUX)
    target_sources(net PRIVATE curl.c)
endif()
