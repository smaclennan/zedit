#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "buff.h"
#include "mark.h"


/* A little over 1k */
const char *str1k =
"Here is what Jeff Foxworthy has to say about Canadians, during "
"a recent appearance at Caesars in Windsor : "
"If someone in a Home Depot store "
"Offers you assistance and they don't work there, "
"You may live in Canada. "
"If you've worn shorts and a parka at the same time, "
"You may live in Canada. "
"If you've had a lengthy telephone conversation "
"With someone who dialed a wrong number, "
"You may live in Canada. "
"If 'Vacation' means going anywhere "
"South of Detroit for the weekend, "
"You may live in Canada. "
"If you measure distance in hours, "
"You may live in Canada. "
"If you know several people "
"Who have hit a deer more than once, "
"You may live in Canada. "
"If you have switched from 'heat' to 'A/C' "
"In the same day and back again, "
"You may live in Canada. "
"If you can drive 90 km/hr through 2 feet of snow "
"During a raging blizzard without flinching, "
"You may live in Canada. "
"If you install security lights on your house and garage, "
"But leave both unlocked, "
"You may live in Canada. "
"If you carry jumper cables in your car "
"And your wife knows how to use them, "
"You may live in Canada.";

void dump_str_at_mark(struct buff *buff, const char *label, struct mark *mrk)
{
	int i;

	printf("%s [%p:%u]: ", label, mrk->mpage, mrk->moffset);

	bswappnt(buff, mrk);
	for (i = 0; i < 16; ++i)
		putchar(*(mrk->mbuff->curcptr + i)); /* cheating since we are in page */
	putchar('\n');
	bswappnt(buff, mrk);
}

int test_readwrite(struct buff *buff, char *in, char *out)
{
	int n, count, fd = open(in, O_RDONLY);
	if (fd < 0) {
		perror(in);
		exit(1);
	}

	while ((n = bread(buff, fd)) > 0) ;

	close(fd);

	if (n < 0) {
		printf("Reads failed\n");
		exit(1);
	}

	btostart(buff);

	fd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		perror(out);
		exit(1);
	}

	do
		count = random() % 4096;
	while ((n = bwrite(buff, fd, count)) > 0);

	close(fd);

	if (n < 0) {
		printf("Writes failed\n");
		exit(1);
	}

	return 0;
}

/* Warning: Leaks memory on error. */
static int test_bigwrite(struct buff *buff, const char *to)
{
	int fd = creat(to, 0644);
	if (fd < 0) {
		perror(to);
		return 1;
	}

	int max = PSIZE * MAX_IOVS * 4;
	char *big = malloc(max);
	if (!big) {
		close(fd);
		puts("Out of memory!");
		return 1;
	}

	memset(big, 'X', 100);
	memset(big + 100, '.', max - 200);
	memset(big + max - 100, 'X', 100);

	bappend(buff, big, max);
	btostart(buff);
	bmove(buff, 100);

	int size = max - 200;
	int n = bwrite(buff, fd, size);
	close(fd);

	if (n != size) {
		printf("Short write %d/%d\n", n, size);
		return 1;
	}

	if ((fd = open(to, O_RDONLY)) < 0) {
		perror(to);
		return 1;
	}
	memset(big, 'S', max);
	n = read(fd, big, size);
	close(fd);

	if (n != size) {
		printf("Short read %d/%d\n", n, size);
		return 1;
	}

	int i;
	for (i = 0; i < size; ++i)
		if (big[i] != '.') {
			printf("%d: error %c\n", i, big[i]);
			return 1;
		}

	free(big);
	puts("Success!");
	return 0;
}

int main(int argc, char *argv[])
{
	struct buff *buff;
	struct mark tmark, tmark1, tmark2, tmark3;
	int total, offset;
	int i, size;

	/* These tests assume a 1k page size */
	assert(PSIZE == 1024);

	buff = bcreate();

	srand(time(NULL));

	if (argc > 2)
		return test_readwrite(buff, argv[1], argv[2]);
	else if (argc == 2)
		return test_bigwrite(buff, argv[1]);

	/* Create a buffer bigger with at least three pages */
	for (i = 0; i < 3; ++i)
		bappend(buff, str1k, strlen(str1k));
	size = blength(buff); /* real size */
	printf("Buffer is %d\n", size);

	return 0;
}

int undo_add_clumped(struct buff *buff, int size) { return 0; }
int undo_del_clumped(struct buff *buff, int size) { return 0; }
