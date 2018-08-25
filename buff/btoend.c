#include "buff.h"

/** Move point to end of buffer. */
void btoend(struct buff *buff)
{
	struct page *lastp;

	/* For huge files we don't want to make every page current */
	for (lastp = buff->curpage; lastp->nextp; lastp = lastp->nextp)
		;
	makecur(buff, lastp, lastp->plen);
}
