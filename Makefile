AS      = nasm
CC      = ia16-elf-gcc
LD      = ia16-elf-ld
OBJCOPY = ia16-elf-objcopy

CFLAGS  = -c -march=i8088 -Os -Iinc/
LDFLAGS = -L/usr/ia16-elf/lib/ -lc -li86 --nmagic -T com.ld

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
include $(ASSOURCES:%.asm=$(OBJ)/%.asm.d)
endif

$(BIN)/%.com: $(OBJ)/%.elf
	@mkdir -p $(BIN)
	$(OBJCOPY) -O binary -j .com $< $@

$(OBJ)/lavender.elf: $(ASSOURCES:%.asm=$(OBJ)/%.asm.o) $(CCSOURCES:%.c=$(OBJ)/%.c.o)
	$(LD) -Map=$(OBJ)/lavender.map -o $@ $^ $(LDFLAGS)

$(OBJ)/data.zip: $(DATA)/*
	zip -0 -r -j $@ $^

$(OBJ)/%.asm.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -w-label-redef-late -iinc/ $<

$(OBJ)/%.c.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ)/%.asm.d: $(SRC)/%.asm
	@mkdir -p $(@D)
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)
