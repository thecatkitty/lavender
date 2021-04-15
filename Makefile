AS      = nasm
LD      = ld


all: lavender.com

%.com: %.pe
	objcopy -O binary $< $@

lavender.pe: lavender.o
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $<

%.o: %.asm
	$(AS) -o $@ -f elf32 -isrc $<

clean:
	rm -f *.com
	rm -f *.pe
	rm -f *.o
