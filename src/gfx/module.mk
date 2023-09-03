ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
CCSOURCES := $(CCSOURCES) $(DIR)/cga.c $(DIR)/vbe.c $(DIR)/font-8x8.c
endif

ifeq ($(LAV_TARGET),windows)
CCSOURCES := $(CCSOURCES) $(DIR)/sdl2.c
endif

ifeq ($(LAV_TARGET),linux)
CCSOURCES := $(CCSOURCES) $(DIR)/sdl2.c
endif
