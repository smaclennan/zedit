/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Delete a buffer freeing all pages and marks.
 * @param buff The buffer to delete. Can be NULL.
 */
void bdelbuff(struct buff *buff)
{
	if (!buff)
		return;

	bhugecleanup(buff);

	while (buff->firstp)	/* delete the pages */
		freepage(buff, buff->firstp);

#ifdef BUFFER_MARKS
	while (buff->marks) /* delete the marks */
		bdelmark(buff->marks);
#endif

	free(buff);	/* free the buffer proper */
}
/* @} */
