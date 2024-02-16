add_library(snd OBJECT)
add_dependencies(snd config)

target_sources(snd PRIVATE snd.c)

if(CONFIG_SOUND)
    target_sources(snd PRIVATE fmidi.c)
    target_sources(snd PRIVATE fspk.c)

    if(DOS)
        target_sources(snd PRIVATE gm-opl2.c)
        target_sources(snd PRIVATE dmpu401.c)
        target_sources(snd PRIVATE dopl2.c)
        target_sources(snd PRIVATE dpcspk.c)
    endif()

    if(WIN32)
        target_sources(snd PRIVATE dmme.c)
    endif()

    if(LINUX)
        target_sources(snd PRIVATE dfluid.c)
        target_sources(snd PRIVATE dpcspk.c)
        target_sources(snd PRIVATE pcspkemu.c)
    endif()
endif()
