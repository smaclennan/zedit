BDIR ?= $(shell uname -m)

CFLAGS += -Wall -g -I.. -DBUFFER_MARKS

all: btest test-bdelete1

btest: btest.c ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

test-bdelete1: test-bdelete1.c ../$(BDIR)/libbuff.a
	$(CC) $(CFLAGS) -o $@ $+

clean:
	rm -f btest test-bdelete1
