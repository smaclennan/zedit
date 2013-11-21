CFLAGS += -Wall

all:	clean zhsort doc2html
	./zhsort c | ./doc2html -H "Appendix A: Commands" > app-a.html
	./zhsort v | ./doc2html -H "Appendix B: Variables" > app-b.html

clean:
	rm -f zhsort doc2html
	rm -f app-[ab].html
