.PHONY: all check install clean

ZEXE = ze

#CC = gcc -std=c11
#CC = clang -fno-color-diagnostics

# Portable C Compiler
#LIBDIR=/usr/local/lib/pcc/x86_64-unknown-linux-gnu/1.2.0.DEVEL/lib
#CC = pcc -D__unix__ -L$(LIBDIR)

# Set this if zlib.h is not in /usr/include
#ZLIBINC=-I/usr/local/include

# Set this if aspell.h is not in /usr/include
#ASPELLINC=-I/usr/local/include

# For pcre
#CFLAGS += -DHAVE_PCRE
#LIBS += -lpcreposix -lpcre

# For HUGE_THREADED
#LIBS += -lpthread

# If you set D=1 on the command line then $(D:1=-g)
# returns -g, else it returns the default (-O2).
D = -O2
CFLAGS += -Wall $(D:1=-g) $(ZLIBINC) $(ASPELLINC) -I. -Ibuff
CFLAGS += -DHAVE_CONFIG_H -include "./config.h"

# Saves about 20k
#CFLAGS += -DNO_HELP

MAKEFLAGS += --no-print-directory

#LIBS += -lz
#LIBS += -ldl
#LIBS += -ltermcap

ETAGS=`which etags || echo true`

CFILES = bcmds.c bind.c cnames.c comment.c commands.c cursor.c delete.c \
	display.c file.c funcs.c getarg.c getfname.c help.c shell.c \
	srch.c tags.c zterm.c vars.c window.c varray.c z.c zgrep.c life.c \
	undo.c

HFILES = config.h funcs.h proto.h vars.h z.h
HFILES += buff/buff.h buff/calc.h buff/mark.h buff/reg.h buff/tinit.h buff/keys.h

O := $(CFILES:.c=.o)

L := bcopyrgn.o bcreate.o bcremark.o bcsearch.o bdelbuff.o \
	bdelete.o bdeltomrk.o bempty.o binsert.o binstr.o \
	bisbeforemrk.o bisaftermrk.o blength.o bline.o blocation.o boffset.o \
	bmovepast.o bmoveto.o bmsearch.o bmove.o bpeek.o \
	bpnttomrk.o breadfile.o bswappnt.o btoend.o bwritefile.o \
	calc.o dbg.o freepage.o globals.o hugefile.o itoa.o kbd.o newpage.o \
	mrkbeforemrk.o mrkaftermrk.o pagesplit.o reg.o strlcpy.o \
	term.o tinit.o tsize.o tstyle.o tobegline.o toendline.o

#################

#
# Pretty print - "borrowed" from sparse Makefile
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

%.o : buff/%.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

#################

all:	fcheck $(ZEXE)

$(ZEXE): $O $L
	$(QUIET_LINK)$(CC) -o $@ $O $L $(LIBS)
	@$(ETAGS) $(CFILES) $(HFILES)

fcheck: fcheck.c funcs.c varray.c cnames.c bind.c config.h vars.h buff/keys.h
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ fcheck.c $(LIBS)
	@./fcheck $(ZLIBINC) $(ASPELLINC)

# Make all c files depend on all .h files
*.o: $(HFILES)

checkit:
	@sparse -D__unix__ -D__linux__ $(CFLAGS) $(CFILES)
	@sparse -D__unix__ -D__linux__ $(CFLAGS) fcheck.c

doxy:
	doxygen doxygen/Doxyfile

install: all
	mkdir -p $(DESTDIR)/bin
	install -s $(ZEXE) $(DESTDIR)/bin/z
#	mkdir -p $(DESTDIR)/usr/share/zedit
#	install -m644 zedit-termcap $(DESTDIR)/usr/share/zedit/termcap

clean:
	rm -f *.o $(ZEXE) fcheck core* TAGS valgrind.out cscope.*
	rm -rf doxygen/html tmpdir
	@$(MAKE) -C docs clean
	@$(MAKE) -C buff clean
