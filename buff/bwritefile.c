#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "buff.h"

#if ZLIB
/** Write out a file compressed. */
static bool bwritegzip(struct buff *buff, int fd)
{
	struct page *tpage;
	bool status = true;
	gzFile gz = gzdopen(fd, "wb");

	if (!gz) {
		close(fd);
		return false;
	}

	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int n = gzwrite(gz, tpage->pdata, tpage->plen);

			status = n == tpage->plen;
		}

	gzclose(gz); /* also closes fd */

	return status;
}
#endif

/** Write out a file normally. */
static bool bwritefd(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = true;

	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(buff, tpage, 0);
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	return status;
}

/** Write out a DOS file. Converts LF to CR LF. */
static bool bwritedos(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = true;
	unsigned i;
	Byte buf[PGSIZE * 2], *p;

	for (tpage = buff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;

			makecur(buff, tpage, 0);
			p = buf;
			for (i = 0; i < tpage->plen; ++i) {
				if (tpage->pdata[i] == '\n') {
					*p++ = '\r';
					++len;
				}
				*p++ = tpage->pdata[i];
			}

			n = write(fd, buf, len);
			status = n == len;
		}

	close(fd);

	return status;
}

/** Write the buffer to the file 'fname'.
 * Mode is umask + FILE_COMPRESSED + FILE_CRLF
 * Returns:	true if write successful.
 * Leaves point at start of buffer.
 */
bool bwritefile(struct buff *buff, char *fname, int mode)
{
	int fd;
	bool status = false;

	if (!fname)
		goto done;

	/* Write the output file */
	fd = creat(fname, mode & 0777);
	if (fd < 0)
		goto done;

#if ZLIB
	if (mode & FILE_COMPRESSED)
		status = bwritegzip(buff, fd);
	else
#endif
		if (mode & FILE_CRLF)
			status = bwritedos(buff, fd);
		else
			status = bwritefd(buff, fd);

	/* cleanup */
	if (status)
		buff->bmodf = false;

done:
	btostart(buff);
	return status;
}