add_library(snd OBJECT)

target_sources(snd PRIVATE snd.c)
target_sources(snd PRIVATE fmidi.c)
target_sources(snd PRIVATE fspk.c)

target_sources(snd PRIVATE dfluid.c)
target_sources(snd PRIVATE dpcspk.c)
target_sources(snd PRIVATE pcspkemu.c)
