.PHONY: all check install clean

ZEXE = ze

CC = cc
#CC = gcc -std=c11
#CC = clang -fno-color-diagnostics

# Portable C Compiler
#LIBDIR=/usr/local/lib/pcc/x86_64-unknown-linux-gnu/1.2.0.DEVEL/lib
#CC = pcc -D__unix__ -L$(LIBDIR)

# Set this if zlib.h is not in /usr/include
#ZLIBINC=-I/usr/local/include

# Set this if aspell.h is not in /usr/include
#ASPELLINC=-I/usr/local/include

# If you set D=1 on the command line then $(D:1=-g)
# returns -g, else it returns the default (-O2).
D = -O2
CFLAGS += -Wall $(D:1=-g) $(ZLIBINC) $(ASPELLINC)
CFLAGS += -DHAVE_CONFIG_H -DHAVE_GLOBAL_MARKS -DUNSIGNED_BYTES

#CFLAGS += -DBUILTIN_REG

MAKEFLAGS += --no-print-directory

# Enable and all buffers have only one contiguous page.
#CFLAGS += -DONE_PAGE

LIBS += -lz
LIBS += -ldl

ETAGS=`which etags || echo true`

CFILES = bcmds.c bind.c buff.c bfile.c calc.c cnames.c \
	comment.c commands.c cursor.c delete.c display.c \
	file.c funcs.c getarg.c getfname.c help.c kbd.c \
	reg.c shell.c spell.c srch.c tags.c term.c mark.c \
	undo.c vars.c window.c varray.c z.c zgrep.c bmsearch.c \
	gpm/liblow.c

HFILES = buff.h config.h funcs.h keys.h proto.h vars.h z.h

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

fcheck: fcheck.c funcs.c kbd.c varray.c cnames.c bind.c config.h vars.h keys.h
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ fcheck.c
	@./fcheck $(ZLIBINC) $(ASPELLINC)

# This is just to check that no zedit dependencies crept into buff.c
main: main.c buff.c bmsearch.c reg.c mark.c bsocket.c
	$(QUIET_LINK)$(CC) -UHAVE_GLOBAL_MARKS -DHAVE_BUFFER_MARKS -g -o $@ $+
	$(QUIET_LINK)$(CC) -UHAVE_GLOBAL_MARKS -UUNSIGNED_BYTES -g -o $@ $+

# Make all c files depend on all .h files
*.o: $(HFILES)

z.o: zversion.h

checkit:
	@sparse -D__unix__ $(CFILES)
	@sparse -D__unix__ fcheck.c

install: all
	mkdir -p $(DESTDIR)/bin
	install -s ze $(DESTDIR)/bin/z

clean:
	rm -f *.o gpm/*.o zversion.h ze fcheck core* TAGS valgrind.out
	@rm -f main mmain tsmain
	@$(MAKE) -C docs clean
