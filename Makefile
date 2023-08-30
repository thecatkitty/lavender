AS      = gcc
CC      = gcc
LD      = gcc
OBJCOPY = objcopy

W32OBJCOPY = i686-w64-mingw32-objcopy
W32WINDRES = i686-w64-mingw32-windres
W64WINDRES = x86_64-w64-mingw32-windres
WINDRES    = $(W64WINDRES)

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

ASFLAGS = -S -Iinc/
CFLAGS  = -c -Os -fno-strict-aliasing -Iinc/
LDFLAGS = -lc -lSDL2 -lSDL2_ttf -no-pie

RESSUFF = .obj
OBJFMT  = elf64-x86-64

ifeq ($(findstring dospc,$(LAV_TARGET)),dospc)
include Makefile.dospc
else
CFLAGS += -g -no-pie
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

$(BINPREF)/lavender$(EXESUFF): $(OBJ)/version.o $(ASSOURCES:%.S=$(OBJPREF)/%.S.o) $(CCSOURCES:%.c=$(OBJPREF)/%.c.o) $(OBJPREF)/resource.$(LAV_LANG)$(RESSUFF)
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
	$(W32OBJCOPY) $< $@ -O $(OBJFMT) --rename-section .rsrc=.rodata.rsrc --add-symbol __w32_rsrc_start=.rodata.rsrc:0

$(OBJPREF)/resource.$(LAV_LANG).obj: $(SRC)/resource.$(LAV_LANG).rc
	$(WINDRES) -c 65001 $< $@ -Iinc/

GIT_TAG     = $(shell git describe --abbrev=0)
GIT_COMMITS = $(shell git rev-list $(GIT_TAG)..HEAD --count)

ifeq ($(GIT_COMMITS),0)
VERSION = $(GIT_TAG)
else
VERSION = $(GIT_TAG)-$(GIT_COMMITS)
endif

$(OBJ)/version.o: $(OBJ)/version.txt .FORCE
	$(OBJCOPY) -I binary -O $(OBJFMT) --rename-section .data=.rodata $< $@

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
