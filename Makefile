.PHONY: all clean zedit xzedit zedit3d

ZEXE=ze
CC ?= gcc
CFLAGS=-O3 -Wall -g $(CDEFS)
MFLAGS+=-rR --no-print-directory

LIBS=-lncurses

FILES=	$D/bcmds.o $D/bind.o $D/bind2.o $D/bindings.o \
	$D/buff.o $D/calc.o $D/comms.o $D/comms1.o \
	$D/cursor.o $D/dbg.o $D/delete.o $D/display.o \
	$D/file.o $D/funcs.o $D/getarg.o $D/global.o \
	$D/help.o $D/kbd.o $D/modes.o\
	$D/reg.o $D/shell.o $D/srch.o $D/support.o \
	$D/tags.o $D/term.o $D/terminfo.o \
	$D/unix.o $D/vars.o $D/window.o $D/z.o \
	$D/getfname.o $D/mbuff.o $D/spell.o $D/make.o \
	$D/comment.o $D/undo.o

XFILES= $D/xinit.o $D/xwind.o \
	$D/xpopup.o $D/xscroll.o $D/socket.o \
	$D/xdbg.o $D/xzoom.o

X3DFILES=$D/3dwindow.o \
	 $D/tk3d.o $D/tkScrollbar.o $D/tkBorders.o $D/xmenu.o

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
cmd_link = $(CC) -o $@ $+ $(LIBS)
do_link = @$(call echo-cmd,link) $(cmd_link)

# No quiet for tags
cmd_tags = etags $+
do_tags =  @$(call echo-cmd,tags) $(cmd_tags)

#################

$D/%.o : %.c
	$(do_dep)
	$(do_cc)

$D/%.o : X/%.c
	$(do_cc)

# WARNING: Full dependencies only work for one target and do not catch
# X/*.c files
#all:	zedit xzedit zedit3d TAGS
all:	zedit TAGS

zedit:
	@$(MAKE) $(MFLAGS) "D=zos" "LIBS=-lncurses" $(ZEXE)

xzedit:
	@$(MAKE) $(MFLAGS) \
		"CDEFS=-DXWINDOWS=1 -DSCROLLBARS -I./X" \
		"D=zxos" \
		"LIBS=-L/usr/X11R6/lib -lX11" \
		x$(ZEXE)

zedit3d:
	@$(MAKE) $(MFLAGS) \
		"CDEFS=-DXWINDOWS=1 -DSCROLLBARS -I./X -DHSCROLL -DBORDER3D" \
		"D=x3d" \
		"LIBS=-L/usr/X11R6/lib -lX11" \
		x3d$(ZEXE)

$(ZEXE): $(FILES)
	$(do_link)

x$(ZEXE): $(FILES) $(XFILES)
	$(CC) $(CFLAGS) -o x$(ZEXE) $(FILES) $(XFILES) $(LIBS)

x3d$(ZEXE): $(FILES) $(XFILES) $(X3DFILES)
	$(CC) $(CFLAGS) -o x3d$(ZEXE) $(FILES) $(XFILES) $(X3DFILES) $(LIBS)

xkey:	xkey.c
	$(CC) -Wall -o xkey xkey.c -L/usr/X11R6/lib -lX11

zfont:	X/zfont.c
	$(CC) -Wall -o zfont X/zfont.c -L/usr/X11R6/lib -lX11

TAGS:	$(wildcard *.c) $(wildcard *.h)
	@$(do_tags)

clean:
	rm -f *.o */*.o .*.dep ze xze x3dze xkey zfont core* TAGS

include $(wildcard .*.dep)
