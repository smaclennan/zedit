#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "buff.h"

int readone(int fd)
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
			return;
		}
		bappend(buffer, n);
		boffset(random() % total);
	} while (n > 0);

	return total;
}

int main(int argc, char *argv[])
{
	struct buff *buff;
	int total, offset;

	buff = bcreate();

	srand(time(NULL));

	if (argc == 1)
		total = readone(0); /* stdin */
	else {
		int fd = open(argv[1], O_RDONLY);
		if (fd < 0) {
			perror(argv[1]);
			exit(1);
		}
		total = readone(fd);
		close(fd);
	}

	if (total < 2) {
		puts("File too small");
		exit(1);
	}

#if 1
	offset = random() % total;
#else
	offset = 2283;
#endif

	char data[2048];
	memset(data, '.', sizeof(data));
	boffset(offset);
	int n = bindata(data, sizeof(data));
	if (n != sizeof(data))
		puts("PROBLEMS");

#if 0
	if (!bwritefile("/tmp/main.test")) {
		printf("Unable to write file\n");
		exit(1);
	}
#endif

#if 0
	binstr("Hello world");
	btostart();
	if (bm_search("hello", false))
		puts("found it!");
	btostart();
	if (bm_search("hello", true))
		puts("Ummm.. shouldn't have found it");

	Byte ep[512];
	if (compile((Byte *)"el.", ep, &ep[512]))
		puts("Failed to compile regexp");
	else {
		btostart();
		step(ep);
	}
#endif

	return 0;
}
