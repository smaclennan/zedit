#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>

static struct termios save_tty;
static struct termios settty;

static void tfini(void)
{
	tcsetattr(fileno(stdin), TCSAFLUSH, &save_tty);
}

/* Initalize the terminal. */
void tinit(void)
{
	tcgetattr(fileno(stdin), &save_tty);
	tcgetattr(fileno(stdin), &settty);
	settty.c_iflag = 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr(fileno(stdin), TCSANOW, &settty);

	atexit(tfini);
}

int main(int argc, char *argv[])
{
	char buff[24];
	int i, n;

	tinit();

	while (1) {
		n = read(0, (char *)buff, sizeof(buff));
		if (n == 0)
			break;

		if (*buff == 'q')
			break;

		for (i = 0; i < n; ++i)
			if (isprint(buff[i]))
				putchar(buff[i]);
			else
				printf("\\%03o", buff[i]);
		puts("\r");
	}

	return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -O3 -Wall keyout.c -o keyout"
 * End:
 */
