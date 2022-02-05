DIR	    := ker
include $(SRC)/$(DIR)/module.mk

DIR	    := crg
include $(SRC)/$(DIR)/module.mk

DIR	    := vid
include $(SRC)/$(DIR)/module.mk

DIR	    := gfx
include $(SRC)/$(DIR)/module.mk

DIR	    := dlg
include $(SRC)/$(DIR)/module.mk

DIR	    := sld
include $(SRC)/$(DIR)/module.mk

CCSOURCES += main.c

DIR	    := tools
include $(SRC)/$(DIR)/module.mk
