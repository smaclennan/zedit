.PHONY: all clean zedit xzedit

ZEXE = ze
CC = gcc
CFLAGS += -O3 -Wall -g $(CDEFS)
#CFLAGS += -pedantic

# For dependencies
CFLAGS += -Wp,-MD,$(@D)/.$(@F).d

MFLAGS += -rR --no-print-directory

#LIBS=-lncurses

FILES=	$D/bcmds.o $D/bind.o $D/bindings.o \
	$D/buff.o $D/calc.o $D/comms.o $D/comms1.o \
	$D/cursor.o $D/dbg.o $D/delete.o $D/display.o \
	$D/file.o $D/funcs.o $D/getarg.o $D/global.o \
	$D/help.o $D/kbd.o $D/modes.o\
	$D/reg.o $D/shell.o $D/srch.o $D/support.o \
	$D/tags.o $D/term.o $D/terminfo.o \
	$D/unix.o $D/vars.o $D/window.o $D/z.o \
	$D/getfname.o $D/spell.o $D/make.o \
	$D/comment.o $D/undo.o $D/ansi.o

XFILES= $D/xinit.o $D/xwind.o $D/xscroll.o $D/xdbg.o $D/xzoom.o

#################

#
# Pretty print - "borrowed" from sparse Makefile
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)

$D/%.o: %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

$D/%.o : X/%.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

#################

ifneq ($(ARCH),)
# It is assumed if ARCH is set you are cross-compiling.
SUFFIX="-$(ARCH)"
CDEFS="-DMINCONFIG"
endif

# WARNING: Full dependencies only work for one target and do not catch
# X/*.c files
#all:	zedit xzedit TAGS
all:	zedit TAGS

zedit:
	@mkdir -p zo$(SUFFIX)
	@$(MAKE) $(MFLAGS) "D=zo$(SUFFIX)" $(ZEXE)

xzedit:
	@mkdir -p zox$(SUFFIX)
	@$(MAKE) $(MFLAGS) \
		"CDEFS=-DXWINDOWS -DSCROLLBARS -I./X" \
		"D=zox$(SUFFIX)" \
		"LIBS=-L/usr/X11R6/lib -lX11" \
		x$(ZEXE)

$(ZEXE): $(FILES)
	$(QUIET_LINK)$(CC) -o $D/$@ $+ $(LIBS)
ifeq ($(ARCH),)
	@rm -f ./ze
	@ln -s zo/ze ./ze
else
	$(do_strip)
endif

x$(ZEXE): $(FILES) $(XFILES)
	$(QUIET_LINK)$(CC) -o $@ $+ $(LIBS)

xkey:	utils/xkey.c
	$(CC) -Wall -o xkey utils/xkey.c -lX11

zfont:	utils/zfont.c
	$(CC) -Wall -o zfont utils/zfont.c -lX11

TAGS:	$(wildcard *.c) $(wildcard *.h)
	@$(do_tags)

clean:
	rm -f *.o */*.o .*.dep ze xze xkey zfont core* TAGS
	rm -rf zo*

include $(wildcard .*.o.d)
