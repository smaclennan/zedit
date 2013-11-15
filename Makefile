.PHONY: all install install-help clean

ZEXE = ze
CC = gcc
#CC = clang -fno-color-diagnostics
CFLAGS += -Wall -g -O3
#CFLAGS += -pedantic

ETAGS=`which etags || echo true`

#LIBS=-lncurses
#LIBS=-ltermcap

CFILES = ansi.c bcmds.c bind.c buff.c calc.c \
	comment.c commands.c cursor.c delete.c display.c \
	file.c funcs.c getarg.c getfname.c help.c kbd.c make.c \
	reg.c shell.c srch.c support.c tags.c term.c \
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

clean:
	rm -f configure.h *.o ze fcheck core* TAGS valgrind.out
	@make -C docs clean
