add_library(snd OBJECT)

target_sources(snd PRIVATE snd.c)
target_sources(snd PRIVATE fmidi.c)
target_sources(snd PRIVATE fspk.c)

if(DOS)
    target_sources(snd PRIVATE gm-opl2.c)
    target_sources(snd PRIVATE dmpu401.c)
    target_sources(snd PRIVATE dopl2.c)
    target_sources(snd PRIVATE dpcspk.c)
endif()

if(LINUX)
    target_sources(snd PRIVATE dfluid.c)
    target_sources(snd PRIVATE dpcspk.c)
    target_sources(snd PRIVATE pcspkemu.c)
endif()
