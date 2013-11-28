.PHONY: all install clean

ZEXE = ze
CC = gcc
#CC = clang -fno-color-diagnostics
CFLAGS += -Wall -g

LIBS += -lz
LIBS += -ldl

ETAGS=`which etags || echo true`

CFILES = bcmds.c bind.c buff.c calc.c \
	comment.c commands.c cursor.c delete.c display.c \
	file.c funcs.c getarg.c getfname.c help.c kbd.c \
	life.c reg.c shell.c spell.c srch.c tags.c term.c \
	undo.c vars.c window.c vars-array.c z.c

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

all:	fcheck $(ZEXE)

$(ZEXE): $O
	$(QUIET_LINK)$(CC) -o $@ $O $(LIBS)
	@$(ETAGS) $(CFILES) *.h

fcheck: fcheck.c funcs.c kbd.c vars-array.c
	$(QUIET_LINK)$(CC) -o $@ fcheck.c $(LIBS)
	@./fcheck

# Make all c files depend on all .h files
*.o: *.h

install:
	mkdir -p $(DESTDIR)/bin
	install -s ze $(DESTDIR)/bin/z

clean:
	rm -f *.o ze fcheck core* TAGS valgrind.out
	@make -C docs clean
