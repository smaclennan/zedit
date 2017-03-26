/* hugefile.c - Deal with huge files
 * Copyright (C) 2016-2017 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

#include "buff.h"

#if HUGE_FILES

#if HUGE_THREADED
static void *read_thread(void *arg);
static void breadpage(struct buff *buff, struct page *page);

#ifdef WIN32
static void do_lock(struct buff *buff)
{
	WaitForSingleObject((HANDLE)read_lock, INFINITE);
}

static void do_unlock(struct buff *buff)
{
	ReleaseMutex((HANDLE)buff->lock);
}

static DWORD WINAPI read_thread_wrapper(void *arg)
{
	read_thread(arg);
	return 0;
}

static void start_thread(struct buff *buff)
{
	DWORD thread;
	HANDLE read_lock;

	read_lock = CreateMutex(NULL, FALSE, NULL);
	if (!read_lock) {
		Dbg("Unable to create mutex");
		return;
	}

	buff->lock = read_lock;

	if (CreateThread(NULL, 0, read_thread_wrapper, buff, 0, &thread) == NULL)
		error("Unable to create thread.");
}
#else
#include <pthread.h>

static void do_lock(struct buff *buff)
{
	if (buff->lock)
		pthread_mutex_lock(buff->lock);
}

static void do_unlock(struct buff *buff)
{
	if (buff->lock)
		pthread_mutex_unlock(buff->lock);
}

static void start_thread(struct buff *buff)
{
	pthread_t thread;

	buff->lock = malloc(sizeof(pthread_mutex_t));
	if (!buff->lock ||
		pthread_mutex_init(buff->lock, NULL) ||
		pthread_create(&thread, NULL, read_thread, buff)) {
		if (buff->lock) {
			free(buff->lock);
			buff->lock = NULL;
		}
		huge_file_cb(buff, EAGAIN);
	}
}
#endif

static void *read_thread(void *arg)
{
	struct buff *buff = arg;
	struct page *page;

	Dbg("read_thread running\n");

	for (page = buff->firstp; page; page = page->nextp)
		if (page->pgoffset)
			breadpage(buff, page);

	bhugecleanup(buff);

	huge_file_cb(buff, 0);

	Dbg("read_thread done\n");

	return NULL;
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
		printf("FATAL I/O Error: page read\r\n");
		exit(2);
	case EBADF:
		printf("FATAL I/O Error: file modified\r\n");
		exit(2);
	default:
		printf("FATAL I/O Error: unexpected error %d\r\n", rc);
		exit(2);
	}
}

void (*huge_file_cb)(struct buff *buff, int rc) = default_huge_file_cb;

static void breadpage(struct buff *buff, struct page *page)
{
	struct stat sbuf;
	unsigned long offset;
	int len;

	do_lock(buff);
	if (page->pgoffset == 0 || buff->fd == -1) {
		do_unlock(buff);
		return;
	}

	offset = page->pgoffset * PGSIZE;
	page->pgoffset = 0;

	if (fstat(buff->fd, &sbuf) ||
		sbuf.st_size != buff->stat->st_size ||
		sbuf.st_mtime != buff->stat->st_mtime)
		goto fatal_mod;
	if (lseek(buff->fd, offset, SEEK_SET) != offset)
		goto fatal;
	len = read(buff->fd, page->pdata, PGSIZE);
	/* All pages should be PGSIZE except the last page */
	if (len != PGSIZE) {
		if (page->nextp)
			goto fatal;
		if (len <= 0)
			goto fatal;
	}

	page->plen = len;
	do_unlock(buff);

#if ! HUGE_THREADED
	if (page->nextp)
		return;

	/* last page - verify all pages read */
	struct page *tp;

	for (tp = buff->firstp; tp; tp = tp->nextp)
		if (tp->pgoffset)
			breadpage(buff, tp);

	close(buff->fd);
	buff->fd = -1;
	huge_file_cb(buff, 0);
#endif
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
	int fd, len, pages, i, rc;
	struct page *page;

	if (buff->fd != -1)
		return EBUSY;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	buff->stat = malloc(sizeof(struct stat));
	if (!buff->stat) {
		rc = ENOMEM;
		goto failed;
	}

	if (fstat(fd, buff->stat)) {
		rc = EIO;
		goto failed;
	}

	bempty(buff);

	/* always read the first page */
	len = read(fd, buff->curcptr, PGSIZE);
	if (len <= 0) {
		rc = EIO;
		goto failed;
	}
	buff->curpage->plen = len;
	buff->fd = fd;

	pages = (buff->stat->st_size + PGSIZE - 1) / PGSIZE;
	page = buff->curpage;
	for (i = 1; i < pages; ++i) {
		page = newpage(page);
		if (!page) {
			bempty(buff); /* will close fd */
			return ENOMEM;
		}
		page->pgoffset = i;
	}

	btostart(buff);

	start_thread(buff);

	return 0;

failed:
	close(fd);
	if (buff->stat)
		free(buff->stat);
	return rc;
}

void makecur(struct buff *buff, struct page *page, int dist)
{
	if (page->pgoffset) {
		breadpage(buff, page);
		if (dist > (int)page->plen)
			dist = page->plen;
	}

	__makecur(buff, page, dist);
}

void bhugecleanup(struct buff *buff)
{
	struct page *page;

	if (buff->fd == -1)
		return; /* normal case */

	do_lock(buff);
	for (page = buff->firstp; page; page = page->nextp)
		page->pgoffset = 0;

	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
	do_unlock(buff);

	if (buff->stat)
		free(buff->stat);
	free(buff->lock);
}
#endif
