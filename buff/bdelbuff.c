#include "buff.h"

/** Delete a buffer. Buffer can be NULL. */
void bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return;

#if HUGE_FILES
	bhugecleanup(tbuff);
#endif

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);

#ifdef HAVE_BUFFER_MARKS
	while (tbuff->marks) /* delete the marks */
		bdelmark(tbuff->marks);
#endif

	free(tbuff);	/* free the buffer proper */
}

