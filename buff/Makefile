CFLAGS += -Wall -g -L.

# For debugging use small pages
#CFLAGS += -DPGSIZE=8 -DPAGE_DBG
# For testing the builtin reg
#CFLAGS += -DBUILTIN_REG
# For hugefiles
#CFLAGS += -DHUGE_FILES

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
	bmovepast.c \
	bmoveto.c \
	bmsearch.c \
	boffset.c \
	bpeek.c \
	bpnttomrk.c \
	breadfile.c \
	bswappnt.c \
	btoend.c \
	bwritefile.c \
	calc.c \
	dbg.c \
	freepage.c \
	globals.c \
	hugefile.c \
	itoa.c \
	kbd.c \
	newpage.c \
	mrkaftermrk.c \
	mrkbeforemrk.c \
	pagesplit.c \
	reg.c \
	strlcpy.c \
	term.c \
	tinit.c \
	tobegline.c \
	toendline.c \

O := $(CFILES:.c=.o)

V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

all: libbuff.a btest

btest: btest.c libbuff.a
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $< -lbuff

libbuff.a: $O
	@rm -f libbuff.a
	$(QUIET_AR)$(AR) cr libbuff.a $O

# The second sparse checks just the buffer code
checkit:
	@sparse -D__unix__ -D__linux__ $(CFLAGS) $(CFILES)

clean:
	rm -f *.o libbuff.a btest sless strtest
