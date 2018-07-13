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
CFLAGS += -Wall $(D:1=-g) $(ZLIBINC) $(ASPELLINC)
CFLAGS += -DHAVE_CONFIG_H

# Saves about 20k
#CFLAGS += -DNO_HELP

MAKEFLAGS += --no-print-directory

#LIBS += -lz
#LIBS += -ldl
#LIBS += -ltermcap

ETAGS=`which etags || echo true`

CFILES = bcmds.c bind.c cnames.c comment.c commands.c cursor.c delete.c \
	display.c file.c funcs.c getarg.c getfname.c help.c shell.c \
	srch.c tags.c term.c vars.c window.c varray.c z.c zgrep.c life.c

LFILES = buff.c bfile.c bmsearch.c mark.c reg.c tinit.c calc.c undo.c \
	kbd.c hugefile.c util.c
# Not used in Zedit
L1FILES=bsocket.c

LHFILES = buff.h calc.h mark.h reg.h tinit.h keys.h

HFILES = config.h funcs.h proto.h vars.h z.h $(LHFILES)

O := $(CFILES:.c=.o)
L := $(LFILES:.c=.o)

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

#################

all:	fcheck $(ZEXE)

$(ZEXE): $O $L
	$(QUIET_LINK)$(CC) -o $@ $O $L $(LIBS)
	@$(ETAGS) $(CFILES) $(LFILES) $(HFILES)

fcheck: fcheck.c funcs.c kbd.c varray.c cnames.c bind.c config.h vars.h keys.h
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ fcheck.c $(LIBS)
	@./fcheck $(ZLIBINC) $(ASPELLINC)

# This is just to check that no zedit dependencies crept into libbuff.a
main: main.c $(LFILES) $(L1FILES)
	@rm -rf tmpdir
	@mkdir tmpdir
	@cp $+ $(LHFILES) tmpdir
	@echo -e "all:\n\t$(CC) -DPGSIZE=1024 -g -o $@ $+" > tmpdir/Makefile
	@make -C tmpdir

# Make all c files depend on all .h files
*.o: $(HFILES)

# The second sparse checks just the buffer code
checkit:
	@sparse -D__unix__ $(CFLAGS) $(CFILES) $(LFILES) $(L1FILES)
	@sparse -D__unix__ $(LFILES) $(L1FILES)
	@sparse -D__unix__ fcheck.c

doxy:
	doxygen doxygen/Doxyfile

install: all
	mkdir -p $(DESTDIR)/bin
	install -s $(ZEXE) $(DESTDIR)/bin/z
#	mkdir -p $(DESTDIR)/usr/share/zedit
#	install -m644 zedit-termcap $(DESTDIR)/usr/share/zedit/termcap

clean:
	rm -f *.o $(ZEXE) fcheck main core* TAGS valgrind.out sless cscope.*
	rm -rf doxygen/html tmpdir
	@$(MAKE) -C docs clean
