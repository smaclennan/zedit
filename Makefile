.PHONY: all install install-help clean

ZEXE = ze
CC = gcc
CFLAGS += -Wall -g
CFLAGS += -pedantic
MAKEFLAGS += --no-print-directory

#LIBS=-lncurses

CFILES = $(wildcard *.c)

#################

#
# Pretty print - "borrowed" from sparse Makefile
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)

%.o: %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

#################

all:	configure.h $(ZEXE)

$(ZEXE): $(CFILES:.c=.o)
	$(QUIET_LINK)$(CC) -o $@ $+ $(LIBS)
	@etags $(CFILES) *.h

configure.h:
	@touch configure.h

# We don't have that many .h files...
# Make all c files depend on all .h files
*.o: *.h

install:
	install ze $(DESTDIR)/bin/z

# You only need this file if HELP is enabled
install-help:
	install -D -m 644 docs/help.z $(DESTDIR)/usr/share/zedit/help.z

clean:
	rm -f configure.h *.o ze core* TAGS valgrind.out
	@make -C docs clean

