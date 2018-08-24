#include "buff.h"

/** Create a buffer and allocate the first page. */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));

	if (buf) {
		buf->firstp = newpage(NULL);
		if (!buf->firstp) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		makecur(buf, buf->firstp, 0);
#if HUGE_FILES
		buf->fd = -1;
#endif
	}
	return buf;
}
