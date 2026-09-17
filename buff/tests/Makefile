BDIR ?= $(shell uname -m)

CFLAGS += -Wall -g -I.. -DBUFFER_MARKS

all: btest

btest: btest.c ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f btest
