/* hugefile.c - Deal with huge files
 * Copyright (C) 2016 Sean MacLennan <seanm@seanm.ca>
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
#include <pthread.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "buff.h"

#if HUGE_FILES
#if ZLIB
#error HUGE_FILES and ZLIB not supported.
#endif

void (*huge_file_cb)(struct buff *buff);

static pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;

/* SAM If we where smart we would read the last partial page and check
 * that all the pages in between where of length PSIZE.
 */
static void breadpage(struct buff *buff, struct page *page)
{
	unsigned long offset;
	int len;

	pthread_mutex_lock(&read_lock);
	if (page->pgoffset == 0 || buff->fd == -1) {
		pthread_mutex_unlock(&read_lock);
		return;
	}

	offset = page->pgoffset * PSIZE;
	page->pgoffset = 0;

	if (lseek(buff->fd, offset, SEEK_SET) != offset)
		goto fatal;
	len = read(buff->fd, page->pdata, PSIZE);
	if (len < 0)
		goto fatal;

	page->plen = len;
	pthread_mutex_unlock(&read_lock);

#if ! HUGE_THREADED
	if (page->nextp)
		return;

	/* last page - verify all pages read */
	struct page *tp;

	for (tp = buff->firstp; tp; tp = tp->nextp)
		if (tp->pgoffset) {
			Dbg("Problem: page offset %u\n", tp->pgoffset);
			breadpage(buff, tp);
		}

	close(buff->fd);
	buff->fd = -1;
	huge_file_cb(buff);
#endif
	return;

fatal:
	printf("\r\nFATAL I/O Error: page %u\r\n", page->pgoffset);
	exit(2);
}

#if HUGE_THREADED
/* We allow only one huge file at a time. */
static pthread_t thread;
static int thread_running;

static void *read_thread(void *arg)
{
	struct buff *buff = arg;
	struct page *page;

	Dbg("read_thread running %d\n", thread_running);

	for (page = buff->firstp; page; page = page->nextp)
		if (page->pgoffset)
			breadpage(buff, page);

	/* Grab lock in case we are closing */
	pthread_mutex_lock(&read_lock);
	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
	pthread_mutex_unlock(&read_lock);

	if (huge_file_cb)
		huge_file_cb(buff);

	thread_running = 0;
	Dbg("read_thread done\n");

	return NULL;
}

static void start_thread(struct buff *buff)
{
	if (!__sync_bool_compare_and_swap(&thread_running, 0, 1))
		return;

	if (pthread_create(&thread, NULL, read_thread, buff)) {
		thread_running = 0;
		Dbg("Unable to create thread\n");
		return;
	}
}
#else
static void start_thread(struct buff *buff) {}
#endif

/* Warning: keeps the fd open. */
int breadhuge(struct buff *buff, int fd, unsigned long size)
{
	int len, pages, i;
	struct page *page;

	/* always read the first page */
	len = read(fd, buff->curcptr, PSIZE);
	if (len <= 0) {
		close(fd);
		return EIO;
	}
	buff->curpage->plen = len;
	buff->fd = fd;

	pages = (size + PSIZE - 1) / PSIZE;
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
}

void makecur(struct buff *buff, struct page *page, int dist)
{
	if (page->pgoffset)
		breadpage(buff, page);
	if (dist > page->plen)
			dist = page->plen;
	buff->curpage = page;
	makeoffset(buff, dist);
}

void bhugecleanup(struct buff *buff)
{
	struct page *page;

	if (buff->fd == -1)
		return; /* normal case */

	pthread_mutex_lock(&read_lock);
	for (page = buff->firstp; page; page = page->nextp)
		page->pgoffset = 0;

	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
	pthread_mutex_unlock(&read_lock);
}
#endif
