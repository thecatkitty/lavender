AS      = nasm
LD      = ld

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

image: $(BIN)/180k.img

$(BIN)/180k.img: $(BIN)/$(SSHOW)
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
