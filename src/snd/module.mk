CCSOURCES := $(CCSOURCES) \
	$(DIR)/snd.c \
	$(DIR)/fmidi.c \
	$(DIR)/fspk.c

ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/dpcspk.c
endif
