BDIR ?= $(shell uname -m)

# Paranoia for Zedit
CFLAGS ?= -Wall -O2

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
	bstrline.c \
	bswappnt.c \
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
	tsize.c \
	tstyle.c \
	tobegline.c \
	toendline.c \

O := $(addprefix $(BDIR)/, $(CFILES:.c=.o))

V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)

$(BDIR)/%.o : %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

all: $(BDIR) $(BDIR)/libbuff.a

btest: btest.c $(BDIR)/libbuff.a
	$(QUIET_LINK)$(CC) $(CFLAGS) -o $@ $+

$(BDIR)/libbuff.a: $O
	@rm -f $(BDIR)/libbuff.a
	$(QUIET_AR)$(AR) cr $@ $O

$(BDIR):
	@mkdir -p $(BDIR)

# The second sparse checks just the buffer code
checkit:
	@sparse -D__unix__ -D__linux__ $(CFLAGS) $(CFILES)

doxy:
	doxygen doxygen/Doxyfile

clean:
	rm -f *.o libbuff.a btest strtest
	rm -rf $(BDIR)
