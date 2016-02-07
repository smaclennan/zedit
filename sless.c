#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "buff.h"
#include "tinit.h"

int rows, cols;

#define bchar (*buff->curcptr)

/* Currently we truncate to cols. */
static void dump_line(struct buff *buff)
{
	int i;

	for (i = 0; i < cols; ++i, bmove1(buff))
		switch (bchar) {
		case '\t':
			/* SAM not quite right... */
			i += 7;
			if (i < cols)
				tputchar('\t');
			break;
		case '\n':
			tputchar('\r');
			tputchar('\n');
			tcleol();
			bmove1(buff);
			return;
		default:
			tputchar(bchar);
	}

	/* Read to NL */
	while (bchar != '\n')
		bmove1(buff);
	bmove1(buff);
}

static void backup(struct buff *buff)
{
	int i, n = rows * 2;

	/* We must go back rows * 2 */
	for (i = 0; i < n && !bisstart(buff); ++i) {
		bmove(buff, -1);
		tobegline(buff);
	}
}

static void goto_line(struct buff *buff, Byte c)
{
	int line = c - '0';

	tputchar(c);
	tflush();
	while ((c = tgetkb()) >= '0' && c <= '9') {
		tputchar(c);
		tflush();
		line = line * 10 + c - '0';
	}

	btostart(buff);
	while (--line > 0)
		bcsearch(buff, '\n');
}

int main(int argc, char *argv[])
{
	if (argc == 0) {
		puts("I need a filename");
		exit(1);
	}

	struct buff *buff = bcreate();
	if (!buff) {
		puts("Unable to create buffer!");
		exit(1);
	}

	if (breadfile(buff, argv[1], NULL)) {
		perror(argv[1]);
		exit(1);
	}

	tinit();
	tsize(&rows, &cols);
	--rows;

	tclrwind();

	int i, cont, needclear;
	Byte c;
	while (!bisend(buff)) {
		for (i = 0; i < rows && !bisend(buff); ++i)
			dump_line(buff);

		t_goto(rows, 0);
		tputchar('>');
		tflush();
		needclear = 0;
		do {
			cont = 1;
			switch (c = tgetkb()) {
			case ' ':
				break;
			case 'b':
				backup(buff);
				break;
			case '0'...'9':
				goto_line(buff, c);
				needclear = 1;
				break;
			case 'q':
				tsetpoint(rows, 0);
				tcleol();
				exit(0);
			default:
				cont = 0;
			}
		} while (!cont);

		tsetpoint(rows, 0);
		tcleol();

		if (needclear)
			tclrwind();
	}

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -g -Wall sless.c buff.c bfile.c tinit.c -o sless"
 * End:
 */
