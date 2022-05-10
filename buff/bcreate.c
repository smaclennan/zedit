/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/** Create a buffer and allocate the first page.
 * @return The newly created buffer or NULL.
 */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));

	if (buf) {
		/* This is the only place that can create a page
		 * without calling newpage().
		 */
		buf->firstp = calloc(1, sizeof(struct page));
		if (!buf->firstp) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		buf->lastp = buf->firstp;
		makecur(buf, buf->firstp, 0);
	}
	return buf;
}

/* @} */
