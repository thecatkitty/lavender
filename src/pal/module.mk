CCSOURCES := $(CCSOURCES) $(DIR)/ziparch.c

ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
ASSOURCES := $(ASSOURCES) $(DIR)/dospc.S
CCSOURCES := $(CCSOURCES) $(DIR)/dospc.c
else
CCSOURCES := $(CCSOURCES) \
	$(DIR)/sdl2arch.c \
	$(DIR)/ziparch.c
endif

ifeq ($(LAV_TARGET),windows)
CCSOURCES := $(CCSOURCES) $(DIR)/windows.c
endif

ifeq ($(LAV_TARGET),linux)
CCSOURCES := $(CCSOURCES) \
	$(DIR)/log.c \
	$(DIR)/linux.c
endif
