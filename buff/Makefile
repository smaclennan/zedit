CFLAGS += -Wall -g -I.. -L.

# For debugging use small pages
CFLAGS += -DPGSIZE=8 -DPAGE_DBG

CFILES = \
	bcreate.c \
	bcsearch.c \
	bdelbuff.c \
	binsert.c \
	bmove.c \
	bmsearch.c \
	breadfile.c \
	bwritefile.c \
	freepage.c \
	globals.c \
	newpage.c \
	pagesplit.c \

O := $(CFILES:.c=.o)

all: libbuff.a btest

btest: btest.c libbuff.a
	$(CC) $(CFLAGS) -o $@ $< -lbuff

libbuff.a: $O
	rm -f libbuff.a
	ar cr libbuff.a $O

clean:
	rm -f *.o libbuff.a btest