DIR	    := pal
include $(SRC)/$(DIR)/module.mk

DIR	    := fmt
include $(SRC)/$(DIR)/module.mk

DIR	    := crg
include $(SRC)/$(DIR)/module.mk

DIR	    := gfx
include $(SRC)/$(DIR)/module.mk

DIR	    := snd
include $(SRC)/$(DIR)/module.mk

DIR	    := dlg
include $(SRC)/$(DIR)/module.mk

DIR	    := sld
include $(SRC)/$(DIR)/module.mk

ASSOURCES += messages.S
CCSOURCES += main.c rstd.c

DIR	    := tools
include $(SRC)/$(DIR)/module.mk
