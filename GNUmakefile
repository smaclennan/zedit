.PHONY: all install install-help clean

ZEXE = ze
CONFIGDIR=/usr/share/zedit
CC = gcc
#CC = clang -fno-color-diagnostics
CFLAGS += -Wall -g -O3 -DCONFIGDIR="\"$(CONFIGDIR)\""
#CFLAGS += -pedantic

ETAGS=`which etags || echo true`

TERMINFO=$(shell awk '/^\#define TERMINFO/ { print $$3 }' config.h)
TERMCAP=$(shell awk '/^\#define TERMCAP/ { print $$3 }' config.h)

ifeq ($(TERMINFO), 1)
LIBS=-lncurses
endif
ifeq ($(TERMCAP), 1)
LIBS=-ltermcap
endif

CFILES = ansi.c bcmds.c bind.c buff.c calc.c \
	comment.c commands.c cursor.c delete.c display.c \
	file.c funcs.c getarg.c getfname.c help.c kbd.c make.c \
	reg.c shell.c spell.c srch.c support.c tags.c term.c \
	termcap.c terminfo.c undo.c unix.c vars.c window.c z.c \
	life.c

O := $(CFILES:.c=.o)

#################

#
# Pretty print - "borrowed" from sparse Makefile
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

#################

all:	configure.h fcheck $(ZEXE)

$(ZEXE): $O
	$(QUIET_LINK)$(CC) -o $@ $O $(LIBS)
	@$(ETAGS) $(CFILES) *.h

configure.h:
	@touch configure.h

fcheck: fcheck.c *.h $(CFILES)
	$(QUIET_LINK)$(CC) -o $@ fcheck.c $(LIBS)
	@./fcheck

# We don't have that many .h files...
# Make all c files depend on all .h files
*.o: *.h

install:
	mkdir -p $(DESTDIR)/bin
	install -s ze $(DESTDIR)/bin/z

install-docs:
	docs/build
	mkdir -p $(DESTDIR)/$(CONFIGDIR)
	install -m 644 docs/help.z $(DESTDIR)$(CONFIGDIR)

clean:
	rm -f configure.h *.o ze fcheck core* TAGS valgrind.out func-docs.h
	@make -C docs clean
