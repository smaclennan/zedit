BDIR ?= $(shell uname -m)

HOST_ARCH := $(shell uname -m)

# Paranoia for Zedit
CFLAGS ?= -Wall -O2

# For debugging use small pages
#CFLAGS += -DPGSIZE=8 -DPAGE_DBG
# For testing the builtin reg
#CFLAGS += -DBUILTIN_REG
# For hugefiles
#CFLAGS += -DHUGE_FILES
# Generally useful
CFLAGS += -DBUFFER_MARKS

CFILES = \
	bcopyrgn.c \
	bcreate.c \
	bcremark.c \
	bcsearch.c \
	bdelbuff.c \
	bdelete.c \
	bdeltomrk.c \
	bempty.c \
	bindata.c \
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
	breadv.c \
	bstrline.c \
	bswappnt.c \
	bwritefile.c \
	bwritev.c \
	calc.c \
	dbg.c \
	freepage.c \
	globals.c \
	hugefile.c \
	mrkaftermrk.c \
	mrkbeforemrk.c \
	newpage.c \
	pagesplit.c \
	reg.c \
	strlcpy.c \
	toendline.c \

O := $(addprefix $(BDIR)/, $(CFILES:.c=.o))

V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_LN      = $(Q:@=@echo    '     LN       '$@;)

$(BDIR)/%.o : %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

all: $(BDIR) $(BDIR)/libbuff.a

tests:
	make -C tests all

$(BDIR)/libbuff.a: $O
	@rm -f $(BDIR)/libbuff.a
	$(QUIET_AR)$(AR) cr $@ $O
ifeq ($(HOST_ARCH), $(BDIR))
	$(QUIET_LN)ln -fs $(BDIR)/libbuff.a libbuff.a
endif

$(BDIR):
	@mkdir -p $(BDIR)

# The second sparse checks just the buffer code
checkit:
	@sparse -D__unix__ -D__linux__ $(CFLAGS) $(CFILES)

doxy:
	doxygen doxygen/Doxyfile

clean:
	$(MAKE) -C tests clean
	rm -f *.o libbuff.a
	rm -rf $(BDIR) doxygen/html
