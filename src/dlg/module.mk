ifeq ($(LAV_TARGET),windows)
CCSOURCES := $(CCSOURCES) $(DIR)/windows.c
else
CCSOURCES := $(CCSOURCES) $(DIR)/fullscrn.c
endif
