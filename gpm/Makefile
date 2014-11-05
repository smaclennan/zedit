# Files from gpm-1.20.7

V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

all: libgpm.a

libgpm.a: liblow.o tools.o
	$(QUIET_AR)ar cr $@ $+

clean:
	rm -f *.o libgpm.a
