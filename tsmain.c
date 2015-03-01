#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "buff.h"

static void do_append(struct buff *buff, char *buffer, int n)
{
#ifdef SAM_NOT_YET
	bappend(buff, buffer, n);
#else
	int i;

	for (i = 0; i < n; ++i, ++buffer)
		_binsert(buff, *buffer);
#endif
}

static int readone(struct buff *buff, int fd)
{
	char buffer[4096];
	int n, max;
	unsigned offset, total = 0;

	do {
		max = random() & 0xfff;
		if (max == 0) max = 1;
		n = read(fd, buffer, max);
		total += n;
		offset = random() % total;
		printf("Read %4d max %4d offset %u/%u\n", n, max, offset, total);
		if (max < 0) {
			perror("read");
			return 0;
		}
		do_append(buff, buffer, n);
		_btostart(buff);
		_bmove(buff, random() % total);
	} while (n > 0);

	return total;
}

int main(int argc, char *argv[])
{
	struct buff *buff;
	int total, offset;

	buff = _bcreate();

	srand(time(NULL));

	if (argc == 1)
		total = readone(buff, 0); /* stdin */
	else {
		int fd = open(argv[1], O_RDONLY);
		if (fd < 0) {
			perror(argv[1]);
			exit(1);
		}
		total = readone(buff, fd);
		close(fd);
	}

	if (total < 2) {
		puts("File too small");
		exit(1);
	}

	return 0;
}
