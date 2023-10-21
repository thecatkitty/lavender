CCSOURCES := $(CCSOURCES) \
	$(DIR)/snd.c \
	$(DIR)/fmidi.c \
	$(DIR)/fspk.c

ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/gm-opl2.c \
	$(DIR)/dmpu401.c \
	$(DIR)/dopl2.c \
	$(DIR)/dpcspk.c
endif

ifeq ($(LAV_TARGET),linux)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/dpcspk.c \
	$(DIR)/dfluid.c \
	$(DIR)/pcspkemu.c
endif

ifeq ($(LAV_TARGET),windows)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/dmme.c
endif
