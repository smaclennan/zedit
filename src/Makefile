ZEXE=ze

MAKE = make
CFLAGS=-O3 -Wall -g $(CDEFS)

#D=.
LIBS=-lncurses

FILES=	$D/bcmds.o $D/bind.o $D/bind2.o $D/bindings.o \
	$D/buff.o $D/calc.o $D/comms.o $D/comms1.o \
	$D/cursor.o $D/dbg.o $D/delete.o $D/display.o \
	$D/file.o $D/funcs.o $D/getarg.o $D/global.o \
	$D/help.o $D/kbd.o $D/life.o $D/macro.o $D/modes.o\
	$D/reg.o $D/shell.o $D/srch.o $D/support.o \
	$D/tags.o $D/term.o $D/termcap.o $D/terminfo.o \
	$D/unix.o $D/vars.o $D/window.o $D/z.o \
	$D/getfname.o $D/mbuff.o $D/spell.o $D/make.o \
	$D/comment.o

XFILES= $D/xinit.o $D/xwind.o \
	$D/xpopup.o $D/xscroll.o $D/socket.o \
	$D/xdbg.o $D/xzoom.o

#$D/adascan.o \$D/cscan.o $D/3dwindow.o
#	$D/tk3d.o $D/tkScrollbar.o $D/tkBorders.o $D/xmenu.o\

$D/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$D/%.o : X/%.c
	$(CC) $(CFLAGS) -c $< -o $@

all:	zedit xzedit

zedit:
	@$(MAKE) $(MFLAGS) "D=zos" "LIBS=-lncurses" $(ZEXE)

xzedit:
	@$(MAKE) $(MFLAGS) \
		"CDEFS=-DXWINDOWS=1 -DSCROLLBARS -I./X" \
		"D=zxos" \
		"LIBS=-L/usr/X11R6/lib -lX11" \
		x$(ZEXE)

zedit3d:
	@$(MAKE) $(MFLAGS) "USE_TERM=0" "USE_X=1" \
		"CDEFS=$(WALL) -I$(OPENWINHOME)/include -DSCROLLBARS -DHSCROLL -DBORDER3D" \
		"D=x3dsun5" \
		"LIBS=-lX11 -lsocket -lnsl" \
		x3dsun5/x$(ZEXE)

$(ZEXE): $(FILES)
	$(CC) $(CFLAGS) -o $(ZEXE) $(FILES) $(LIBS)

x$(ZEXE): $(FILES) $(XFILES)
	$(CC) $(CFLAGS) -o x$(ZEXE) $(FILES) $(XFILES) $(LIBS)

$D/funcs.o: cnames.h

# memlog won't compile under gcc
memlog.o: memlog.c
	cc $(CFLAGS) -c memlog.c

xkey:	xkey.c
	$(CC) -Wall -o xkey xkey.c -L/usr/X11R6/lib -lX11

zfont:	zfont.c
	$(CC) -Wall -o zfont zfont.c -L/usr/X11R6/lib -lX11

ctags:
	tagfiles

lint:
	lint $(SFILES) > lint.out 2>&1

clean:
	rm -f *.o */*.o ze */ze xze */xze xkey zfont core TAGS
