CFLAGS += -Wall -O2 -I.. -L.

CFILES = newpage.c

O := $(CFILES:.c=.o)

all: libbuff.a btest

btest: btest.c
	$(CC) $(CFLAGS) -o $@ $< -lbuff

libbuff.a: $O
	rm -f libbuff.a
	ar cr libbuff.a $O

clean:
	rm -f *.o libbuff.a btest
