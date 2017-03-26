#include <stdio.h>
#include <ctype.h>
#include "tinit.h"

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
 * compile-command: "gcc -O3 -Wall keyout.c tinit.c -o keyout"
 * End:
 */
