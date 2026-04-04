BDIR ?= $(shell uname -m)

CFLAGS += -Wall -g -I.. -DBUFFER_MARKS

all: strtest keyout

strtest: strtest.c ../$(BDIR)/liblibz.a
	$(CC) $(CFLAGS) -o $@ $+

keyout: keyout.c  ../$(BDIR)/liblibz.a
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f strtest keyout
