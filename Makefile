.PHONY: all clean zedit xzedit zedit3d

ZEXE = ze
CC ?= gcc
CFLAGS += -O3 -Wall -g $(CDEFS)
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

# Default to quiet
quiet = quiet_

# make V=1 shows all commands
ifdef V
  ifeq ("$(origin V)", "command line")
	quiet=
  endif
endif

# make -s is completely silent
ifneq ($(findstring s,$(MAKEFLAGS)),)
  quiet=silent_
endif

echo-cmd = $(if $($(quiet)cmd_$(1)),echo '  $($(quiet)cmd_$(1))';)

quiet_cmd_cc = CC $<
cmd_cc = $(CC) $(CFLAGS) -c -o $@ $<
do_cc = @$(call echo-cmd,cc) $(cmd_cc)

cmd_dep = $(CC) -M -MM -MT $@ $(CFLAGS) -o .$<.dep $<
do_dep = @$(call echo-cmd,dep) $(cmd_dep)

quiet_cmd_link = LD $@
cmd_link = $(CC) -o $D/$@ $+ $(LIBS)
do_link = @$(call echo-cmd,link) $(cmd_link)

quiet_cmd_strip = STRIP $@
cmd_strip = $(CROSS_COMPILE)strip $D/$@
do_strip = @$(call echo-cmd,strip) $(cmd_strip)

# No quiet for tags
cmd_tags = etags $+
do_tags =  @$(call echo-cmd,tags) $(cmd_tags)

#################

ifneq ($(ARCH),)
# It is assumed if ARCH is set you are cross-compiling.
SUFFIX="-$(ARCH)"
CDEFS="-DMINCONFIG"
endif

$D/%.o : %.c
	$(do_dep)
	$(do_cc)

$D/%.o : X/%.c
	$(do_cc)

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
	$(do_link)
ifeq ($(ARCH),)
	@rm -f ./ze
	@ln -s zo/ze ./ze
else
	$(do_strip)
endif

x$(ZEXE): $(FILES) $(XFILES)
	$(do_link)
	@rm -f ./xze
	@ln -s zox/xze ./xze

xkey:	utils/xkey.c
	$(CC) -Wall -o xkey utils/xkey.c -lX11

zfont:	utils/zfont.c
	$(CC) -Wall -o zfont utils/zfont.c -lX11

TAGS:	$(wildcard *.c) $(wildcard *.h)
	@$(do_tags)

clean:
	rm -f *.o */*.o .*.dep ze xze xkey zfont core* TAGS
	rm -rf zo*

include $(wildcard .*.dep)
