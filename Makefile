AS      = ia16-elf-gcc
ASM     = nasm
CC      = ia16-elf-gcc
LD      = ia16-elf-ld
OBJCOPY = ia16-elf-objcopy

ASFLAGS = -c -march=i8088 -Iinc/ -Wa,--divide
CFLAGS  = -c -march=i8088 -Os -Iinc/
LDFLAGS = -L/usr/lib/x86_64-linux-gnu/gcc/ia16-elf/6.3.0 -L/usr/ia16-elf/lib -T com.ld -li86 --nmagic

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
include $(ASMSOURCES:%.asm=$(OBJ)/%.asm.d)
endif

$(BIN)/lavender.com: $(ASMSOURCES:%.asm=$(OBJ)/%.asm.o) $(ASSOURCES:%.S=$(OBJ)/%.S.o) $(CCSOURCES:%.c=$(OBJ)/%.c.o)
	@mkdir -p $(BIN)
	$(LD) -Map=$(OBJ)/lavender.map -o $@ $^ $(LDFLAGS)

$(OBJ)/data.zip: $(DATA)/*
	zip -0 -r -j $@ $^

$(OBJ)/%.S.o: $(SRC)/%.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJ)/%.asm.o: $(SRC)/%.asm
	$(ASM) -o $@ -f elf32 -w-label-redef-late -iinc/ $<

$(OBJ)/%.c.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJ)/%.asm.d: $(SRC)/%.asm
	@mkdir -p $(@D)
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)
