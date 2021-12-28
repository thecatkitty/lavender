AS      = nasm
LD      = ia16-elf-ld
OBJCOPY = ia16-elf-objcopy

BIN     = bin
OBJ     = obj
SRC     = src

ifdef LAV_DATA
DATA    = $(LAV_DATA)
else
DATA    = data
endif

ifdef LAV_SSHOW
SSHOW   = $(LAV_SSHOW)
else
SSHOW   = sshow.com
endif

include sources.mk

$(BIN)/$(SSHOW): $(BIN)/lavender.com $(OBJ)/data.zip
	cat $^ > $@
	@if [ $$(stat -L -c %s $@) -gt 65280 ]; then echo >&2 "'$@' size exceedes 65,280 bytes"; false; fi

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:%.asm=$(OBJ)/%.d)
endif

$(BIN)/%.com: $(OBJ)/%.elf
	@mkdir -p $(BIN)
	$(OBJCOPY) -O binary -j .com $< $@

$(OBJ)/lavender.elf: $(SOURCES:%.asm=$(OBJ)/%.o)
	$(LD) --nmagic -T com.ld -o $@ $^

$(OBJ)/data.zip: $(DATA)/*
	zip -0 -r -j $@ $^

$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -w-label-redef-late -iinc/ $<

$(OBJ)/%.d: $(SRC)/%.asm
	@mkdir -p $(@D)
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)
