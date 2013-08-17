.PHONY: all clean

ZEXE = ze
CC = gcc
CFLAGS += -Wall -g
CFLAGS += -pedantic

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

all:	$(ZEXE)

$(ZEXE): $(CFILES:.c=.o)
	$(QUIET_LINK)$(CC) -o $@ $+ $(LIBS)
	@etags $(CFILES) *.h

# We don't have that many .h files...
# Make all c files depend on all .h files
*.o: *.h

clean:
	rm -f *.o ze core* TAGS
