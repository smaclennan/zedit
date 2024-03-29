/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include <fcntl.h>
#include <errno.h>
#include "buff.h"

/* \cond skip */
#if ZLIB
#undef Byte
#include <zlib.h>

#define fileread(a, b, c) gzread(gz, b, c)
#define fileclose(a) gzclose(gz)
#else
#define fileread(a, b, c) read(a, b, c)
#define fileclose(a) close(a)
#endif
/* \endcond */

/** @addtogroup buffer
 * @{
 */

/**
 * Insert a file into the buffer at the point.
 * Leaves the point at the start of the inserted file.
 * @param buff The buffer to read the file into.
 * @param fname The file to read.
 * @param[out] compressed Was the file compressed? Only if ZLIB enabled.
 *             Can be NULL.
 * @return 0 on success, -errno on general errors, 1 on zlib error.
 */
int breadfile(struct buff *buff, const char *fname, int *compressed)
{
	char buf[PGSIZE];
	struct mark start;
	int fd, len;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return -errno;

#if ZLIB
	gzFile gz = gzdopen(fd, "rb");

	if (!gz) {
		close(fd);
		return 1;
	}

	if (compressed)
		*compressed = gzdirect(gz) == 0;
#else
	if (compressed)
		*compressed = 0;
#endif

	/* Deal with point in the middle of a page. */
	if (curplen(buff) && buff->curchar < PGSIZE) {
		pagesplit(buff, buff->curchar);
		if (!newpage(buff))
			return -ENOMEM;
		makecur(buff, buff->curpage->nextp, 0);
	}

	bmrktopnt(buff, &start);

	while ((len = fileread(fd, buf, PGSIZE)) > 0) {
		if (curplen(buff)) {
			if (!newpage(buff)) {
				fileclose(fd);
				return -ENOMEM;
			}
			makecur(buff, buff->curpage->nextp, 0);
		}
		memcpy(buff->curcptr, buf, len);
		curplen(buff) = len;
	}
	fileclose(fd);

	bpnttomrk(buff, &start);

#if ZLIB
	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (compressed && curplen(buff) == 0)
		*compressed = 0;
#endif

	buff->bmodf = 0;

	return 0;
}
/* @} */
