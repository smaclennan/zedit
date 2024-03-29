/* Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca> */

#include <fcntl.h>
#include "buff.h"

/** @addtogroup buffer
 * @{
 */

/* \cond skip */
#if ZLIB
/** Write out a file compressed. */
static int bwritegzip(struct buff *buff, int fd)
{
	struct page *tpage;
	int status = 1;
	gzFile gz = gzdopen(fd, "wb");

	if (!gz) {
		close(fd);
		return 0;
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
static int bwritefd(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = 1;

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
static int bwritedos(struct buff *buff, int fd)
{
	struct page *tpage;
	int n, status = 1;
	unsigned int i;
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
/* \endcond */

/** Write the buffer to the file. If mode FILE_COMPRESSED is
 * set, then file is written out compressed (it did not have to be
 * read compressed). If mode FILE_CRLF is set, then the file is
 * written out in DOS mode.
 * Leaves point at start of buffer.
 * @param buff The buffer to write out.
 * @param fname  The file to write out to.
 * @param mode umask + FILE_COMPRESSED + FILE_CRLF
 * @return 0 on success, negative number on error.
 */
int bwritefile(struct buff *buff, char *fname, int mode)
{
	int fd;
	int status = 0;

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
		buff->bmodf = 0;

done:
	btostart(buff);
	return status;
}
/* @} */
