export GIT_COMMIT = $(shell git rev-parse --short HEAD)
export GIT_TAG    = $(shell git describe --tags $(GIT_COMMIT) --abbrev=0)

ASSOURCES := $(ASSOURCES) $(DIR)/start.asm $(DIR)/error.asm $(DIR)/info.asm $(DIR)/time.asm $(DIR)/string.asm $(DIR)/unicode.asm $(DIR)/zip.asm $(DIR)/messages.asm
CCSOURCES := $(CCSOURCES) $(DIR)/int.c
