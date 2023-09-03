CCSOURCES := $(CCSOURCES) \
	$(DIR)/snd.c \
	$(DIR)/fmidi.c \
	$(DIR)/fspk.c \
	$(DIR)/dpcspk.c

ifeq ($(findstring test,$(LAV_TARGET)),test)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/pcspkemu.c \
	$(DIR)/dfluid.c
endif
