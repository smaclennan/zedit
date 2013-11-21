CFLAGS += -Wall

all:	clean zhsort
	./zhsort -cH "Appendix A: Commands" > app-a.html
	./zhsort -vH "Appendix B: Variables" > app-b.html

clean:
	rm -f zhsort app-[ab].html
