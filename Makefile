AS      = ia16-elf-gcc
CC      = ia16-elf-gcc
LD      = ia16-elf-ld
OBJCOPY = ia16-elf-objcopy

W32OBJCOPY = i686-w64-mingw32-objcopy
W32WINDRES = i686-w64-mingw32-windres

BIN     = bin
OBJ     = obj
SRC     = src

ifndef LAV_LANG
LAV_LANG = ENU
endif

ifndef LAV_TARGET
LAV_TARGET = dospc-exe
endif

BINPREF = $(BIN)/$(LAV_TARGET)/$(LAV_LANG)
OBJPREF = $(OBJ)/$(LAV_TARGET)

ASFLAGS = -S -march=i8088  -Iinc/
CFLAGS  = -c -march=i8088 -Os -Wall -Werror -fno-strict-aliasing -Iinc/

ifeq ($(LAV_TARGET),dospc-exe)
LD      = ia16-elf-gcc
CFLAGS += -mcmodel=small
LDFLAGS = -mcmodel=small -li86 -Wl,--nmagic -Wl,-Map=$(BINPREF)/lavender.map
EXESUFF = .exe
endif

ifeq ($(LAV_TARGET),dospc-com)
CFLAGS += -DZIP_PIGGYBACK
LDFLAGS = -L/usr/lib/x86_64-linux-gnu/gcc/ia16-elf/6.3.0 -L/usr/ia16-elf/lib -T com.ld -li86 --nmagic -Map=$(BINPREF)/lavender.map
EXESUFF = .com
endif

ifdef LAV_DATA
DATA    = $(LAV_DATA)
else
DATA    = data
endif

ifdef LAV_SSHOW
SSHOW   = $(LAV_SSHOW)
else
SSHOW   = sshow$(EXESUFF)
endif

sshow: $(BINPREF)/$(SSHOW)

include sources.mk

$(BINPREF)/$(SSHOW): $(BINPREF)/lavender$(EXESUFF) $(OBJ)/data.zip
	cat $^ > $@
	@if [ $$(stat -L -c %s $@) -gt 65280 ]; then echo >&2 "'$@' size exceeds 65,280 bytes"; false; fi

$(BINPREF)/lavender$(EXESUFF): $(OBJ)/version.o $(ASSOURCES:%.S=$(OBJPREF)/%.S.o) $(CCSOURCES:%.c=$(OBJPREF)/%.c.o) $(OBJPREF)/resource.$(LAV_LANG).o
	@mkdir -p $(BINPREF)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJ)/data.zip: $(DATA)/*
	zip -0 -r -j $@ $^

$(OBJPREF)/%.S.o: $(SRC)/%.S
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJPREF)/%.c.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(OBJPREF)/resource.$(LAV_LANG).o: $(OBJPREF)/resource.$(LAV_LANG).obj
	@mkdir -p $(@D)
	$(W32OBJCOPY) $< $@ -O elf32-i386 --rename-section .rsrc=.rodata.rsrc --add-symbol __w32_rsrc_start=.rodata.rsrc:0

$(OBJPREF)/resource.$(LAV_LANG).obj: $(SRC)/resource.$(LAV_LANG).rc
	$(W32WINDRES) -c 65001 $< $@ -Iinc/

GIT_TAG     = $(shell git describe --abbrev=0)
GIT_COMMITS = $(shell git rev-list $(GIT_TAG)..HEAD --count)

ifeq ($(GIT_COMMITS),0)
VERSION = $(GIT_TAG)
else
VERSION = $(GIT_TAG)-$(GIT_COMMITS)
endif

$(OBJ)/version.o: $(OBJ)/version.txt .FORCE
	$(OBJCOPY) -I binary -O elf32-i386 -B i386 --rename-section .data=.rodata $< $@

$(OBJ)/version.txt: .FORCE
	@mkdir -p $(OBJ)
	/bin/echo -en "Lavender $(VERSION)\x00" >$@

 .FORCE:

clean:
	rm -rf $(BINPREF)
	rm -rf $(OBJPREF)

clean-all:
	rm -rf $(BIN)
	rm -rf $(OBJ)
