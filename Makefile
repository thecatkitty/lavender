AS      = nasm
LD      = ld

BIN     = bin
OBJ     = obj
SRC     = src

SOURCES = err.asm str.asm cga.asm zip.asm lineexec.asm lineread.asm lavender.asm

export GIT_COMMIT = $(shell git rev-parse --short HEAD)
export GIT_TAG = $(shell git describe --tags $(GIT_COMMIT) --abbrev=0)

$(BIN)/vii.com: $(BIN)/lavender.com $(OBJ)/data.zip
	cat $^ > $@

image: $(BIN)/180k.img

$(BIN)/180k.img: $(BIN)/vii.com
	dd if=/dev/zero of=$@ bs=512 count=360 status=none
	mformat -i $@ -f 180 -v lavender
	touch -m -t 201004100841 $^
	mcopy -m -i $@ $^ ::

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:%.asm=$(OBJ)/%.d)
endif

$(BIN)/%.com: $(OBJ)/%.pe
	@mkdir -p $(BIN)
	objcopy -O binary -j .com $< $@

$(OBJ)/lavender.pe: $(SOURCES:%.asm=$(OBJ)/%.o)
	$(LD) -m i386pe --nmagic -T com.ld -o $@ $^

$(OBJ)/data.zip: data/*
	zip -0 -r -j $@ $^

$(OBJ)/%.o: $(SRC)/%.asm
	$(AS) -o $@ -f elf32 -iinc/ $<

$(OBJ)/%.d: $(SRC)/%.asm
	@mkdir -p $(OBJ)
	@rm -f $@; \
	 cat $< | grep '^\s*%include' | sed -r 's,.+"(.+)".*,inc/\1,g' | tr '\n' ' ' | sed -r 's,^,$(@:.d=.o) $@ : $< ,g' > $@

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)
