add_library(net OBJECT)
target_link_libraries(net PRIVATE compat)

if(WIN32)
    target_sources(net PRIVATE wininet.c)
endif()

if(LINUX)
    target_sources(net PRIVATE curl.c)
endif()
