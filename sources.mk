DIR	    := ker
include $(SRC)/$(DIR)/module.mk

DIR	    := vid
include $(SRC)/$(DIR)/module.mk

DIR	    := gfx
include $(SRC)/$(DIR)/module.mk

DIR	    := sld
include $(SRC)/$(DIR)/module.mk

ASSOURCES += main.asm
