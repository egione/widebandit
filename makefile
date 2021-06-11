BASICFLAGS=-D_FILE_OFFSET_BITS=64 -fno-stack-protector -mtune=native
BITS=32
CC=cc
ifeq (,$(findstring which:,$(shell which gcc)))
	CC=gcc
endif
DELTEMP=rm tmp$(SLASH)*
EXE=
NASM=
OBJ=.o
OPTIMIZEFLAGS=-O3
SLASH=/
TARGET=$(shell $(CC) -dumpmachine)
WARNFLAGS=-Wall -Wextra -Wconversion
ifneq (,$(findstring mingw,$(TARGET)))
	DELTEMP=@echo clean does not work due to idiotic problems with MAKE under MSYS, but deleting everything in the tmp subfolder accomplishes the same thing.
	EXE=.exe
	OBJ=.obj
	SLASH=\\
endif
ifneq (,$(findstring 64,$(TARGET)))
	BITS=64
	WARNFLAGS+= -Wint-conversion
endif
ifneq (,$(findstring darwin,$(TARGET)))
	WARNFLAGS+= -Wconstant-conversion
else
ifneq (,$(findstring arm,$(TARGET)))
	WARNFLAGS+= -Wconstant-conversion
endif
endif

archive:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)archive$(OBJ) archive.c

archive_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)archive$(OBJ) archive.c

ascii:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)ascii$(OBJ) ascii.c

ascii_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)ascii$(OBJ) ascii.c

bitscan:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)bitscan$(OBJ) bitscan.c

clean:
	$(DELTEMP)

debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)debug$(OBJ) debug.c

downsample:
	make archive
	make ascii
	make emit
	make fakefloat
	make filesys
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)downsample$(EXE) downsample.c

downsample_debug:
	make archive_debug
	make ascii_debug
	make debug
	make emit
	make fakefloat_debug
	make filesys_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)downsample$(EXE) downsample.c

emit:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)emit$(OBJ) emit.c

fakefloat:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fakefloat$(OBJ) fakefloat.c

fakefloat_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fakefloat$(OBJ) fakefloat.c

filesys:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)filesys$(OBJ) filesys.c

filesys_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)filesys$(OBJ) filesys.c

fracterval_u128:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u128$(OBJ) fracterval_u128.c

fracterval_u128_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u128$(OBJ) fracterval_u128.c

fracterval_u64:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u64$(OBJ) fracterval_u64.c

fracterval_u64_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)fracterval_u64$(OBJ) fracterval_u64.c

gaussify:
	make archive
	make ascii
	make emit
	make fakefloat
	make filesys
	make gic
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gaussify$(EXE) -lm gaussify.c

gaussify_debug:
	make archive_debug
	make ascii_debug
	make debug
	make emit
	make fakefloat_debug
	make filesys_debug
	make gic_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gaussify$(EXE) -lm gaussify.c

gic:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)gic$(OBJ) gic.c -lm

gic_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)gic$(OBJ) gic.c -lm

gicrank:
	make ascii
	make emit
	make filesys
	make gic
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gicrank$(EXE) -lm gicrank.c

gicrank_debug:
	make ascii_debug
	make debug
	make emit
	make filesys_debug
	make gic_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gicrank$(EXE) -lm gicrank.c

slice:
	make archive
	make ascii
	make emit
	make fakefloat
	make filesys
	make whole
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)slice$(EXE) slice.c

slice_debug:
	make archive_debug
	make ascii_debug
	make debug
	make emit
	make fakefloat_debug
	make filesys_debug
	make whole_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)slice$(EXE) slice.c

skan:
	make ascii
	make bitscan
	make emit
	make filesys
	make fracterval_u128
	make fracterval_u64
	make transform
	make whole
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)transform$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)skan$(EXE) skan.c

skan_debug:
	make ascii_debug
	make bitscan
	make debug
	make emit
	make filesys_debug
	make fracterval_u128_debug
	make fracterval_u64_debug
	make transform_debug
	make whole_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)transform$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)skan$(EXE) skan.c

spectrafy:
	make archive
	make ascii
	make emit
	make fakefloat
	make filesys
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)spectrafy$(EXE) spectrafy.c

spectrafy_debug:
	make archive_debug
	make ascii_debug
	make debug
	make emit
	make fakefloat_debug
	make filesys_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)spectrafy$(EXE) spectrafy.c

transform:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)transform$(OBJ) transform.c

transform_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)transform$(OBJ) transform.c

whole:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)whole$(OBJ) whole.c

whole_debug:
	$(CC) -c -fpic -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) -otmp$(SLASH)whole$(OBJ) whole.c

widebandit:
	make archive
	make ascii
	make bitscan
	make emit
	make fakefloat
	make filesys
	make fracterval_u128
	make fracterval_u64
	make gic
	make transform
	make whole
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)downsample$(EXE) downsample.c
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gaussify$(EXE) -lm gaussify.c
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gicrank$(EXE) -lm gicrank.c
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)transform$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)skan$(EXE) skan.c
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)slice$(EXE) slice.c
	$(CC) -D_$(BITS)_ -DDEBUG_OFF $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)spectrafy$(EXE) spectrafy.c

widebandit_debug:
	make archive_debug
	make ascii_debug
	make bitscan
	make debug
	make emit
	make fakefloat_debug
	make filesys_debug
	make fracterval_u128_debug
	make fracterval_u64_debug
	make gic_debug
	make transform_debug
	make whole_debug
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)downsample$(EXE) downsample.c
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gaussify$(EXE) -lm gaussify.c
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)gic$(OBJ) -otmp$(SLASH)gicrank$(EXE) -lm gicrank.c
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)bitscan$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)fracterval_u128$(OBJ) tmp$(SLASH)fracterval_u64$(OBJ) tmp$(SLASH)transform$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)skan$(EXE) skan.c
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) tmp$(SLASH)whole$(OBJ) -otmp$(SLASH)slice$(EXE) slice.c
	$(CC) -D_$(BITS)_ -DDEBUG $(BASICFLAGS) $(OPTIMIZEFLAGS) $(WARNFLAGS) tmp$(SLASH)archive$(OBJ) tmp$(SLASH)ascii$(OBJ) tmp$(SLASH)debug$(OBJ) tmp$(SLASH)emit$(OBJ) tmp$(SLASH)fakefloat$(OBJ) tmp$(SLASH)filesys$(OBJ) -otmp$(SLASH)spectrafy$(EXE) spectrafy.c
