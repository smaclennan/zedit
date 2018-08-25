CFLAGS += -Wall -g -I.. -L.

# For debugging use small pages
CFLAGS += -DPGSIZE=8 -DPAGE_DBG
# For testing the builtin reg
#CFLAGS += -DBUILTIN_REG

CFILES = \
	bcopyrgn.c \
	bcreate.c \
	bcremark.c \
	bcsearch.c \
	bdelbuff.c \
	bdelete.c \
	bdeltomrk.c \
	bempty.c \
	binsert.c \
	binstr.c \
	bisbeforemrk.c \
	bisaftermrk.c \
	blength.c \
	bline.c \
	blocation.c \
	bmove.c \
	bmsearch.c \
	boffset.c \
	bpeek.c \
	bpnttomrk.c \
	breadfile.c \
	bswappnt.c \
	btoend.c \
	bwritefile.c \
	dbg.c \
	freepage.c \
	globals.c \
	newpage.c \
	mrkaftermrk.c \
	mrkbeforemrk.c \
	pagesplit.c \
	reg.c \
	strlcpy.c \
	tobegline.c \
	toendline.c \

O := $(CFILES:.c=.o)

all: libbuff.a btest

btest: btest.c libbuff.a
	$(CC) $(CFLAGS) -o $@ $< -lbuff

libbuff.a: $O
	rm -f libbuff.a
	ar cr libbuff.a $O

clean:
	rm -f *.o libbuff.a btest
