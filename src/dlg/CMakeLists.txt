add_library(dlg OBJECT)

if(WIN32)
    target_sources(dlg PRIVATE windows.c)
    target_sources(dlg PRIVATE prompt.rc)
else()
    target_sources(dlg PRIVATE fullscrn.c)
endif()
