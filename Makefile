.PHONY: all check install clean

ZEXE = ze

CC = cc
#CC = gcc
#CC = g++
#CC = clang -fno-color-diagnostics

# Set this if zlib.h is not in /usr/include
#ZLIBINC=-I/usr/include

# Set this if aspell.h is not in /usr/include
#ASPELLINC=-I/usr/include

# If you set D=1 on the command line then $(D:1=-g)
# returns -g, else it returns the default (-O2).
D = -O2
CFLAGS += -Wall $(D:1=-g) $(ZLIBINC) $(ASPELLINC)

# Enable and all buffers have only one contiguous page.
#CFLAGS += -DONE_PAGE

LIBS += -lz
LIBS += -ldl

ETAGS=`which etags || echo true`

CFILES = bcmds.c bind.c buff.c calc.c cnames.c \
	comment.c commands.c cursor.c delete.c display.c \
	file.c funcs.c getarg.c getfname.c help.c kbd.c \
	reg.c shell.c spell.c srch.c tags.c term.c \
	undo.c vars.c window.c varray.c z.c zgrep.c

O := $(CFILES:.c=.o)

GIT_COMMIT=$(shell cat .git/refs/heads/master)
GIT_MOD=$(shell git status --porcelain | fgrep -v '??' | wc -l)

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

z.o:	z.c
	$(QUIET_CC)$(CC) -c $(CFLAGS) -DGIT_COMMIT=\"$(GIT_COMMIT)\" -DGIT_MOD=$(GIT_MOD) $<

fcheck: fcheck.c funcs.c kbd.c varray.c cnames.c bind.c vars.h keys.h
	$(QUIET_LINK)$(CC) -o $@ fcheck.c $(LIBS)
	@./fcheck $(ZLIBINC) $(ASPELLINC)

# Make all c files depend on all .h files
*.o: *.h

check:
	@sparse -D__unix__ $(CFILES)
	@sparse -D__unix__ fcheck.c

install: all
	mkdir -p $(DESTDIR)/bin
	install -s ze $(DESTDIR)/bin/z

clean:
	rm -f *.o ze fcheck core* TAGS valgrind.out
	@$(MAKE) -C docs clean
