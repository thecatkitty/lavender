add_library(dlg OBJECT)

if(WIN32)
    target_sources(dlg PRIVATE windows.c)
else()
    target_sources(dlg PRIVATE fullscrn.c)
endif()
