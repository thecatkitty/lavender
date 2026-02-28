add_library(snd OBJECT)

target_sources(snd PRIVATE drivers.c)
target_sources(snd PRIVATE snd.c)
target_sources(snd PRIVATE buf.c)
target_sources(snd PRIVATE seq.c)

function(add_sound_driver drv_name config_var target_var)
    if("${${config_var}}" STREQUAL m)
        add_driver(${drv_name})
        set(${target_var} ${drv_name} PARENT_SCOPE)
    else()
        set(${target_var} snd PARENT_SCOPE)
    endif()
endfunction()

if(CONFIG_SOUND_BEEP)
    add_sound_driver(sndbeep CONFIG_SOUND_BEEP tgt)
    target_sources(${tgt} PRIVATE dbeep.c)
endif()

if(CONFIG_SOUND_BEEPEMU)
    target_sources(snd PRIVATE beepemu.c)
endif()

if(CONFIG_SOUND_FLUID)
    target_sources(snd PRIVATE dfluid.c)
endif()

if(CONFIG_SOUND_MPU401)
    add_sound_driver(sndmpu CONFIG_SOUND_MPU401 tgt)
    target_sources(${tgt} PRIVATE dmpu401.c)
endif()

if(CONFIG_SOUND_OPL2)
    add_sound_driver(sndopl CONFIG_SOUND_OPL2 tgt)
    target_sources(${tgt} PRIVATE gm-opl2.c dopl2.c)
endif()

if(CONFIG_SOUND_WINMM)
    target_sources(snd PRIVATE dmme.c)
endif()
