CCSOURCES := $(CCSOURCES) \
	$(DIR)/snd.c \
	$(DIR)/fmidi.c \
	$(DIR)/fspk.c \
	$(DIR)/dpcspk.c

ifeq ($(findstring linux,$(LAV_TARGET)),linux)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/pcspkemu.c \
	$(DIR)/dfluid.c
endif
