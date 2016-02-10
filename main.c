#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

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
	int total, offset;

	buff = bcreate();

	srand(time(NULL));

	if (argc > 2)
		return test_readwrite(buff, argv[1], argv[2]);
	else if (argc == 2)
		return test_bigwrite(buff, argv[1]);

	/* Testing bcsearch/bcrsearch */
	bappend(buff, str1k, strlen(str1k));
	btostart(buff);
	if (!bcsearch(buff, 'C'))
		printf("Did not find C\n");
	else if (*buff->curcptr != 'a')
		printf("Problems with bcsearch\n");
	else {
		bmove(buff, 10);
		if (!bcrsearch(buff, 'C'))
			printf("Did not reverse find C\n");
		else if (*buff->curcptr != 'C')
			printf("Problems with bcrsearch\n");
	}

	/* testing pagesplit */
	bappend(buff, (Byte *)str1k, 800); /* over half page */
	btostart(buff);
	if (!bm_search(buff, "wrong number", true)) {
		puts("search1 failed");
		exit(1);
	}
	bmove(buff, -12);
	struct mark *mark1 = bcremark(buff);

	bmove(buff, 12 + 11 + 10);
	if (!bm_rsearch(buff, "wrong number", true)) {
		puts("search1r failed");
		exit(1);
	}
	printf("Buffer is %c\n", *buff->curcptr);

	if (!bm_search(buff, "2 feet of snow", true)) {
		puts("search2 failed");
		exit(1);
	}
	bmove(buff, -14);
	struct mark *mark2 = bcremark(buff);

	if (mark1->moffset >= HALFP || mark2->moffset <= HALFP) {
		puts("PROBLEMS");
		exit(1);
	}

	dump_str_at_mark(buff, "1", mark1);
	dump_str_at_mark(buff, "2", mark2);

	pagesplit(buff, 600);

	dump_str_at_mark(buff, "1", mark1);
	dump_str_at_mark(buff, "2", mark2);

	btostart(buff);
#if 1
	/* Here is what Jeff Foxworthy has to say about Canadians, during
	 * ^
	 */
	if (lookingat(buff, "is what")) puts("la probs 1");
	bmove(buff, 4);
	/* Here is what Jeff Foxworthy has to say about Canadians, during
	 *     ^
	 */
	if (lookingat(buff, "is what")) puts("la probs 2");
	bmove1(buff);
#else
	bmove(buff, 5);
#endif
	/* Here is what Jeff Foxworthy has to say about Canadians, during
	 *      ^
	 */
	if (!lookingat(buff, "is what")) puts("la probs 3");
	btostart(buff);
	bmove(buff, 5);
	if (lookingat(buff, "is what nope")) puts("la probs 4");
	if (!lookingat(buff, "is what")) puts("la probs 6");

	bempty(buff);
	int n = 1234;
	binstr(buff, "%-32s %8u\n", "fred", n);
	binstr(buff, "%15s %8d\n", "fred", n);
	binstr(buff, "%-15s %8u\n", "fredrick the first", n);
	binstr(buff, "%-15s left\n", "fredrick the first");
	printf("%.*s", curplen(buff), buff->curpage->pdata);

	printf("%-15s %8u\n", "fred", n);

	tinit();

	return 0;
}

int undo_add_clumped(struct buff *buff, int size) { return 0; }
int undo_del_clumped(struct buff *buff, int size) { return 0; }
