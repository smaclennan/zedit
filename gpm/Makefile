# Files from gpm-1.20.7

all: libgpm.a

libgpm.a: liblow.o report-lib.o tools.o
	ar cr $@ $+

clean:
	rm -f *.o libgpm.a
