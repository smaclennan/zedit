.PHONY: all check install clean libbuff

ZEXE = ze

HOST_ARCH := $(shell uname -m)

# The build directory
BDIR ?= $(HOST_ARCH)

#CC = gcc -std=c11
#CC = clang -fno-color-diagnostics

# Set this if zlib.h is not in /usr/include
#ZLIBINC=-I/usr/local/include

# Set this if aspell.h is not in /usr/include
#ASPELLINC=-I/usr/local/include

# For pcre
#CFLAGS += -DHAVE_PCRE
#LIBS += -lpcreposix -lpcre

# For HUGE_THREADED
#LIBS += -lsamthread

# If you set D=1 on the command line then $(D:1=-g)
# returns -g, else it returns the default (-O2).
D = -O2
CFLAGS += -Wall $(D:1=-g) $(ZLIBINC) $(ASPELLINC) -I. -Ibuff -DHAVE_CONFIG_H

# Saves about 20k
#CFLAGS += -DNO_HELP

MFLAGS += --no-print-directory CC="$(CC)" CFLAGS="$(CFLAGS) -I.." BDIR="z-$(BDIR)"

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

O := $(addprefix $(BDIR)/, $(CFILES:.c=.o))

#################

#
# Pretty print - "borrowed" from sparse Makefile
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_MAKE    = $(Q:@=@echo    '   MAKE       '$@;)

$(BDIR)/%.o : %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

#################

all:	fcheck $(BDIR) $(BDIR)/$(ZEXE)

$(BDIR)/$(ZEXE): $O libbuff
	$(QUIET_LINK)$(CC) -o $@ $O buff/z-$(BDIR)/libbuff.a $(LIBS)
ifeq ($(BDIR), $(HOST_ARCH))
	@rm -f $(ZEXE)
	@ln -s $(BDIR)/$(ZEXE) $(ZEXE)
endif
	@$(ETAGS) $(CFILES) buff/*.c $(HFILES)

libbuff:
	$(QUIET_MAKE)$(MAKE) $(MFLAGS) -C buff

fcheck: fcheck.c funcs.c varray.c cnames.c bind.c config.h vars.h buff/keys.h
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ fcheck.c $(LIBS)
	@./fcheck $(ZLIBINC) $(ASPELLINC)

$(BDIR):
	@mkdir -p $(BDIR)

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
	rm -rf doxygen/html tmpdir $(BDIR)
	@$(MAKE) -C docs clean
	@$(MAKE) $(MFLAGS) -C buff clean
	rm -rf buff/$(BDIR)
