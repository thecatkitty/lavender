AS      = nasm
LD      = ld

BIN     = bin
OBJ     = obj
SRC     = src

SOURCES = err.asm str.asm cga.asm zip.asm lineexec.asm lineread.asm lavender.asm

all: $(BIN)/vii.com

$(BIN)/vii.com: $(BIN)/lavender.com $(OBJ)/data.zip
	cat $^ > $@

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:%.asm=$(OBJ)/%.d)
endif

$(BIN)/%.com: $(OBJ)/%.pe
	objcopy -O binary -j .com $< $@

$(OBJ)/lavender.pe: $(SOURCES:%.asm=$(OBJ)/%.o)
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $^

$(OBJ)/data.zip: data/*
	zip -0 -r -j $@ $^

$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -iinc $<

$(OBJ)/%.d: $(SRC)/%.asm
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -f bin/*.com
	rm -f obj/*.*
