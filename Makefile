AS      = nasm
LD      = ld

BIN     = bin
OBJ     = obj
SRC     = src


all: bin/lavender.com

$(BIN)/%.com: $(OBJ)/%.pe
	objcopy -O binary -j .com $< $@

$(OBJ)/lavender.pe: $(OBJ)/cga.o \
					$(OBJ)/strings.o \
					$(OBJ)/lineexec.o \
					$(OBJ)/lineread.o \
					$(OBJ)/lavender.o
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $^

$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -isrc $<

clean:
	rm -f bin/*.com
	rm -f obj/*.*
