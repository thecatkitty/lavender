ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
CCSOURCES := $(CCSOURCES) $(DIR)/cga.c $(DIR)/vbe.c $(DIR)/font-8x8.c
endif
