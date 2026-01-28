add_library(pal OBJECT)
target_include_directories(pal PRIVATE inc)

add_subdirectory(_zip)

if(DOS)
    add_subdirectory(dos)
endif()

if(WIN32)
    add_subdirectory(windows)
endif()

if(LINUX)
    add_subdirectory(linux)
    target_sources(pal PRIVATE log.c)
endif()
