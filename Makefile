AS      = nasm
LD      = ld

BIN     = bin
OBJ     = obj
SRC     = src

SOURCES = cga.asm strings.asm lineexec.asm lineread.asm lavender.asm

all: $(BIN)/sshow.com

$(BIN)/sshow.com: $(BIN)/lavender.com data/slides.txt
	cat $^ > $(BIN)/sshow.com

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:%.asm=$(OBJ)/%.d)
endif

$(BIN)/%.com: $(OBJ)/%.pe
	objcopy -O binary -j .com $< $@

$(OBJ)/lavender.pe: $(SOURCES:%.asm=$(OBJ)/%.o)
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $^
	
$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -iinc $<

$(OBJ)/%.d: $(SRC)/%.asm
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -f bin/*.com
	rm -f obj/*.*
