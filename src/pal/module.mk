CCSOURCES := $(CCSOURCES) $(DIR)/ziparch.c

ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
ASSOURCES := $(ASSOURCES) $(DIR)/dospc.S
CCSOURCES := $(CCSOURCES) $(DIR)/dospc.c
endif

ifeq ($(LAV_TARGET),linux)
CCSOURCES := $(CCSOURCES) $(DIR)/log.c $(DIR)/linux.c
endif
