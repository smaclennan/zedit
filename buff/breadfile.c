#include <fcntl.h>
#include <errno.h>
#include "buff.h"

#if ZLIB
#undef Byte
#include <zlib.h>

#define fileread(a, b, c) gzread(gz, b, c)
#define fileclose(a) gzclose(gz)
#else
#define fileread(a, b, c) read(a, b, c)
#define fileclose(a) close(a)
#endif

/**
 * Load the file 'fname' into the current buffer at the point.  Returns 0
 * successfully opened file, < 0 (errno) on error, 1 on gzdopen
 * error.
 * Leaves point at start of inserted file.
 */
int breadfile(struct buff *buff, const char *fname, int *compressed)
{
	char buf[PGSIZE];
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
		if (!newpage(buff->curpage))
			return -ENOMEM;
		makecur(buff, buff->curpage->nextp, 0);
	}

	while ((len = fileread(fd, buf, PGSIZE)) > 0) {
		if (curplen(buff)) {
			if (!newpage(buff->curpage)) {
				fileclose(fd);
				return -ENOMEM;
			}
			makecur(buff, buff->curpage->nextp, 0);
		}
		memcpy(buff->curcptr, buf, len);
		curplen(buff) = len;
	}
	fileclose(fd);

#if ZLIB
	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (compressed && curplen(buff) == 0)
		*compressed = 0;
#endif

	buff->bmodf = false;

	return 0;
}