add_library(net OBJECT)

if(WIN32)
    target_sources(net PRIVATE wininet.c)
endif()

if(LINUX)
    target_sources(net PRIVATE curl.c)
endif()
