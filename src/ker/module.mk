export GIT_COMMIT = $(shell git rev-parse --short HEAD)
export GIT_TAG    = $(shell git describe --tags $(GIT_COMMIT) --abbrev=0)

SOURCES := $(SOURCES) $(DIR)/init.asm $(DIR)/error.asm $(DIR)/string.asm $(DIR)/unicode.asm $(DIR)/zip.asm
