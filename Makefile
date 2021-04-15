AS      = nasm
LD      = ld

BIN     = bin
OBJ     = obj
SRC     = src

SOURCES = cga.asm strings.asm lineexec.asm lineread.asm lavender.asm

all: bin/lavender.com

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:%.asm=$(OBJ)/%.d)
endif

$(BIN)/%.com: $(OBJ)/%.pe
	objcopy -O binary -j .com $< $@

$(OBJ)/lavender.pe: $(SOURCES:%.asm=$(OBJ)/%.o)
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $^
	
$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -isrc $<

$(OBJ)/%.d: $(SRC)/%.asm
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,$(SRC)/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -f bin/*.com
	rm -f obj/*.*
