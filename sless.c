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
#include "keys.h"
#include "reg.h"

static int rows, cols;

#define bchar (*buff->curcptr)

static int dump_line(struct buff *buff, int row)
{
	int i, did = 1;

	while (1) {
		for (i = 0; i < cols; ++i, bmove1(buff))
			switch (bchar) {
			case '\t':
				i += 8 - (i & (8 - 1)) - 1; /* we increment 1 */
				tputchar('\t');
				break;
			case '\n':
				tputchar('\r');
				tputchar('\n');
				tcleol();
				bmove1(buff);
				return did;
			default:
				tputchar(bchar);
			}

		tputchar('\r');
		tputchar('\n');
		tcleol();

		if (++row < rows)
			++did;
		else
			return did;
	}
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

static int get_line(char *line, int len)
{
	int c, i = 0;

	while ((c = tgetkb()) != '\r')
		switch (c) {
		case ' '...'~':
			tputchar(c);
			tflush();
			if (i < len - 1)
				line[i++] = c;
			break;
		case 0177: /* backspace */
			if (i > 0) {
				tputchar('\b');
				tputchar(' ');
				tputchar('\b');
				tflush();
				--i;
			}
			break;
		case 03: /* ctrl-c */
		case 07: /* ctrl-g */
			tsetpoint(rows, 0);
			tcleol();
			return 1;
		case 04: /* ctrl-d */
			tsetpoint(rows, 0);
			tcleol();
			exit(0);
		}

	if (i > 0)
		line[i] = 0;
	return 0;
}

static int do_search(struct buff *buff)
{
	static char search[80];
	regexp_t re;

	tputchar('\b');
	tputchar('/');
	tflush();
	if (get_line(search, sizeof(search)))
		return 1;

	if (re_compile(&re, search, REG_EXTENDED))
		return 1;

	return re_step(buff, &re, NULL) ? 0 : 1;
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		fputs("I need a filename\n", stderr);
		exit(1);
	}

	struct buff *buff = bcreate();
	if (!buff) {
		fputs("Unable to create buffer!\n", stderr);
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
		for (i = 0; i < rows && !bisend(buff); )
			i += dump_line(buff, i);

	again:
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
			case 04: /* ctrl-d */
				tsetpoint(rows, 0);
				tcleol();
				exit(0);
			case '/':
				if (do_search(buff))
					goto again;
				needclear = 1;
				tobegline(buff);
				break;
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
 * compile-command: "gcc -g -Wall sless.c buff.c mark.c bfile.c tinit.c kbd.c dbg.c reg.c -o sless"
 * End:
 */
