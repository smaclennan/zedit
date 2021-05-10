BDIR ?= $(shell uname -m)

CFLAGS += -Wall -g -I.. -DBUFFER_MARKS

all: btest strtest

btest: btest.c ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

strtest: strtest.c ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

keyout: keyout.c  ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f btest strtest keyout
