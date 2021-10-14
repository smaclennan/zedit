/* Deal with huge files
 * Copyright (C) 2016-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "buff.h"
#include "tinit.h"

#if HUGE_FILES

#if HUGE_THREADED
#include <samthread.h>

static void breadpage(struct buff *buff, struct page *page);

static void do_lock(struct buff *buff)
{
	if (buff->lock)
		mutex_lock(buff->lock);
}

static void do_unlock(struct buff *buff)
{
	if (buff->lock)
		mutex_unlock(buff->lock);
}

static int read_thread(void *arg)
{
	struct buff *buff = arg;
	struct page *page;

	Dbg("%s running\n", __func__);

	for (page = buff->firstp; page; page = page->nextp)
		if (page->pgoffset)
			breadpage(buff, page);

	bhugecleanup(buff);

	huge_file_cb(buff, 0);

	Dbg("%s done\n", __func__);

	return 0;
}

static void start_thread(struct buff *buff)
{
	buff->lock = mutex_create();
	if (!buff->lock)
		goto failed;

	if (samthread_create(read_thread, buff) == (samthread_t)-1)
		goto failed;

	return;

failed:
	mutex_destroy(buff->lock);
	buff->lock = NULL;
	huge_file_cb(buff, EAGAIN);
}

#else
#define do_lock(b)
#define do_unlock(b)
#define start_thread(b)
#endif

void default_huge_file_cb(struct buff *buff, int rc)
{
	switch (rc) {
	case 0:
	case EAGAIN:
		return; /* success */
	case EIO:
		terror("FATAL I/O Error: page read\n");
		break;
	case EBADF:
		terror("FATAL I/O Error: file modified\n");
		break;
	default:
		terror("FATAL I/O Error: unexpected error\n");
		break;
	}
	exit(2);
}

void (*huge_file_cb)(struct buff *buff, int rc) = default_huge_file_cb;

static void breadpage(struct buff *buff, struct page *page)
{
	struct stat sbuf;
	unsigned long offset;
	int len;

	do_lock(buff);
	if (page->pgoffset == 0 || buff->huge == NULL) {
		do_unlock(buff);
		return;
	}

	offset = page->pgoffset * PGSIZE;
	page->pgoffset = 0;

	if (fstat(buff->huge->fd, &sbuf) ||
		sbuf.st_size != buff->huge->stat.st_size ||
		sbuf.st_mtime != buff->huge->stat.st_mtime)
		goto fatal_mod;
	if (lseek(buff->huge->fd, offset, SEEK_SET) != offset)
		goto fatal;
	len = read(buff->huge->fd, page->pdata, PGSIZE);
	/* All pages should be PGSIZE except the last page */
	if (len != PGSIZE) {
		if (page->nextp)
			goto fatal;
		if (len <= 0)
			goto fatal;
	}

	page->plen = len;
	--buff->huge->n_huge;
	do_unlock(buff);

	if (buff->huge->n_huge <= 0) {
		/* last page - verify all pages read */
		struct page *tp;

		for (tp = buff->firstp; tp; tp = tp->nextp)
			if (tp->pgoffset)
				breadpage(buff, tp);

		huge_file_cb(buff, 0);
		close(buff->huge->fd);
		free(buff->huge);
		buff->huge = NULL;
	}

	return;

fatal:
	huge_file_cb(buff, EIO);
	return; /* assume they dealt with it */
fatal_mod:
	huge_file_cb(buff, EBADF);
}

/* Warning: keeps the fd open. */
int breadhuge(struct buff *buff, const char *fname)
{
	int fd, len, pages, i, rc, left;
	struct page *page;

	if (buff->huge)
		return -EBUSY;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return -errno;

	buff->huge = calloc(1, sizeof(struct huge_file));
	if (!buff->huge) {
		rc = -ENOMEM;
		goto failed;
	}

	if (fstat(fd, &buff->huge->stat)) {
		rc = -EIO;
		goto failed;
	}

	bempty(buff);

	/* always read the first page */
	len = read(fd, buff->curcptr, PGSIZE);
	if (len <= 0) {
		rc = -EIO;
		goto failed;
	}
	buff->curpage->plen = len;
	buff->huge->fd = fd;

	pages = buff->huge->stat.st_size / PGSIZE;
	left = buff->huge->stat.st_size % PGSIZE;
	page = buff->curpage;
	buff->huge->n_huge = pages - 1;
	for (i = 1; i < pages; ++i) {
		page = newpage(page);
		if (!page) {
			bempty(buff); /* will close fd */
			return -ENOMEM;
		}
		page->pgoffset = i;
		page->plen = PGSIZE;
	}

	if (left) {
		page = newpage(page);
		if (!page) {
			bempty(buff); /* will close fd */
			return -ENOMEM;
		}
		page->pgoffset = i;
		page->plen = left;
		++buff->huge->n_huge;
	}

	btostart(buff);

	start_thread(buff);

	return 0;

failed:
	close(fd);
	free(buff->huge);
	buff->huge = NULL;
	return rc;
}

void makecur(struct buff *buff, struct page *page, int dist)
{
	if (page->pgoffset) {
		breadpage(buff, page);
		if (dist > (int)page->plen)
			dist = page->plen;
	}

	buff->curpage = page;
	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

void bhugecleanup(struct buff *buff)
{
	struct page *page;

	if (buff->huge == NULL)
		return; /* normal case */

	do_lock(buff);
	for (page = buff->firstp; page; page = page->nextp)
		page->pgoffset = 0;

	if (buff->huge->fd >= 0)
		close(buff->huge->fd);
	do_unlock(buff);

	free(buff->huge);
}
#endif
