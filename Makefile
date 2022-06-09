AS      = ia16-elf-gcc
CC      = ia16-elf-gcc
LD      = ia16-elf-ld
OBJCOPY = ia16-elf-objcopy

ifndef LAV_LANG
LAV_LANG = ENU
endif

ASFLAGS = -c -march=i8088 -Iinc/ -DLANG=LCID_$(LAV_LANG) -Wa,--divide
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

sshow: $(BIN)/$(SSHOW)

include sources.mk

$(BIN)/$(SSHOW): $(BIN)/lavender.com $(OBJ)/data.zip
	cat $^ > $@
	@if [ $$(stat -L -c %s $@) -gt 65280 ]; then echo >&2 "'$@' size exceeds 65,280 bytes"; false; fi

$(BIN)/lavender.com: $(OBJ)/version.o $(ASSOURCES:%.S=$(OBJ)/%.S.o) $(CCSOURCES:%.c=$(OBJ)/%.c.o)
	@mkdir -p $(BIN)
	$(LD) -Map=$(OBJ)/lavender.map -o $@ $^ $(LDFLAGS)

$(OBJ)/data.zip: $(DATA)/*
	zip -0 -r -j $@ $^

$(OBJ)/%.S.o: $(SRC)/%.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJ)/%.c.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

GIT_TAG     = $(shell git describe --abbrev=0)
GIT_COMMITS = $(shell git rev-list $(GIT_TAG)..HEAD --count)

ifeq ($(GIT_COMMITS),0)
VERSION = $(GIT_TAG)
else
VERSION = $(GIT_TAG)-$(GIT_COMMITS)
endif

$(OBJ)/version.o: $(OBJ)/version.txt .FORCE
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 --rename-section .data=.startupA.1 $< $@

$(OBJ)/version.txt: .FORCE
	@mkdir -p $(OBJ)
	/bin/echo -en "\rLavender $(VERSION)\x1A" >$@

 .FORCE:

clean:
	rm -rf $(BIN)
	rm -rf $(OBJ)
