#include <stdio.h>
#include <ctype.h>
#include "tinit.h"

int main(int argc, char *argv[])
{
	unsigned char buff[24];
	int i, n;

	tinit();

	while (1) {
		n = read(0, buff, sizeof(buff));
		if (n == 0)
			break;

		if (*buff == 'q')
			break;

		for (i = 0; i < n; ++i)
			if (buff[i] >= ' ' && buff[i] <= '~')
				putchar(buff[i]);
			else
				printf("\\%03o", buff[i]);
		puts("\r");
	}

	return 0;
}
