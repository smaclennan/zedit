#include "buff.h"

/** Move the point to a given absolute offset in the buffer. */
void boffset(struct buff *buff, unsigned long offset)
{
	struct page *tpage;

	/* find the correct page */
	for (tpage = buff->firstp; tpage->nextp; tpage = tpage->nextp) {
		if (tpage->plen >= offset)
			break;
		offset -= tpage->plen;
	}

	makecur(buff, tpage, offset);
}
