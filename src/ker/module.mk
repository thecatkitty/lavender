export GIT_COMMIT = $(shell git rev-parse --short HEAD)
export GIT_TAG    = $(shell git describe --tags $(GIT_COMMIT) --abbrev=0)

ASSOURCES := $(ASSOURCES) $(DIR)/start.asm $(DIR)/error.asm $(DIR)/string.asm $(DIR)/zip.asm $(DIR)/messages.asm
CCSOURCES := $(CCSOURCES) $(DIR)/info.c $(DIR)/int.c $(DIR)/time.c $(DIR)/unicode.c
