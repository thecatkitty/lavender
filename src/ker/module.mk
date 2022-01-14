export GIT_COMMIT = $(shell git rev-parse --short HEAD)
export GIT_TAG    = $(shell git describe --tags $(GIT_COMMIT) --abbrev=0)

ASSOURCES := $(ASSOURCES) $(DIR)/messages.S
ASMSOURCES := $(ASMSOURCES) $(DIR)/start.asm
CCSOURCES := $(CCSOURCES) $(DIR)/error.c $(DIR)/info.c $(DIR)/int.c $(DIR)/time.c $(DIR)/unicode.c $(DIR)/zip.c
