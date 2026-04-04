BDIR ?= $(shell uname -m)

HOST_ARCH := $(shell uname -m)

# Paranoia for Zedit
CFLAGS ?= -Wall -O2

# For testing the builtin reg
#CFLAGS += -DBUILTIN_REG

CFILES = \
	calc.c \
	dbg.c \
	globals.c \
	kbd.c \
	strlcpy.c \
	term.c \
	tinit.c \
	tsize.c \
	tstyle.c \

O := $(addprefix $(BDIR)/, $(CFILES:.c=.o))

V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_LN      = $(Q:@=@echo    '     LN       '$@;)

$(BDIR)/%.o : %.c
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

all: $(BDIR) $(BDIR)/liblibz.a

tags:
	etags *.[ch]

tests:
	make -C tests all

$(BDIR)/liblibz.a: $O
	@rm -f $(BDIR)/liblibz.a
	$(QUIET_AR)$(AR) cr $@ $O
ifeq ($(HOST_ARCH), $(BDIR))
	$(QUIET_LN)ln -fs $(BDIR)/liblibz.a liblibz.a
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
	rm -f *.o liblibz.a
	rm -rf $(BDIR) doxygen/html
