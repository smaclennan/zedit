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
#include <signal.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "buff.h"

#if HUGE_FILES

#ifndef O_BINARY
#define O_BINARY 0
#endif

#if HUGE_THREADED
#ifdef WIN32
static HANDLE read_lock = NULL;

static void do_lock(void)
{
	if (read_lock == NULL) {
		read_lock = CreateMutex(NULL, FALSE, NULL);
		if (!read_lock) {
			Dbg("Unable to create mutex");
			return;
		}
	}

	WaitForSingleObject(read_lock, INFINITE);
}

static void do_unlock(void)
{
	if (read_lock)
		ReleaseMutex(read_lock);
}

static void *read_thread(void *arg);

static DWORD WINAPI read_thread_wrapper(void *arg)
{
	read_thread(arg);
	return 0;
}

static void start_thread(struct buff *buff)
{
	DWORD thread;

	if (CreateThread(NULL, 0, read_thread_wrapper, buff, 0, &thread) == NULL)
		Dbg("Unable to create thread.");
}
#else
#include <pthread.h>

static pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;

static void do_lock(void) { pthread_mutex_lock(&read_lock); }
static void do_unlock(void) { pthread_mutex_unlock(&read_lock); }

static void *read_thread(void *arg);

static void start_thread(struct buff *buff)
{
	pthread_t thread;

	if (pthread_create(&thread, NULL, read_thread, buff))
		Dbg("Unable to create thread\n");
}
#endif
#else
#define do_lock()
#define do_unlock()
#define start_thread(b)
#endif

void (*huge_file_cb)(struct buff *buff);

static void breadpage(struct buff *buff, struct page *page)
{
	struct stat sbuf;
	unsigned long offset;
	int len;

	do_lock();
	if (page->pgoffset == 0 || buff->fd == -1) {
		do_unlock();
		return;
	}

	offset = page->pgoffset * PSIZE;
	page->pgoffset = 0;

	if (fstat(buff->fd, &sbuf) || sbuf.st_size != buff->size)
		goto fatal_mod;
	if (lseek(buff->fd, offset, SEEK_SET) != offset)
		goto fatal;
	len = read(buff->fd, page->pdata, PSIZE);
	/* All pages should be PSIZE except the last page */
	if (len != PSIZE) {
		if (page->nextp)
			goto fatal;
		if (len <= 0)
			goto fatal;
	}

	page->plen = len;
	do_unlock();

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
	if (huge_file_cb)
		huge_file_cb(buff);
#endif
	return;

fatal:
	printf("\033[2JFATAL I/O Error: page %u\r\n", page->pgoffset);
	exit(2);
fatal_mod:
	printf("\033[2JFATAL I/O Error: file modified\r\n");
	exit(2);
}

#if HUGE_THREADED
static void *read_thread(void *arg)
{
	struct buff *buff = arg;
	struct page *page;

	Dbg("read_thread running\n");

	for (page = buff->firstp; page; page = page->nextp)
		if (page->pgoffset)
			breadpage(buff, page);

	/* Grab lock in case we are closing */
	do_lock();
	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
	do_unlock();

	if (huge_file_cb)
		huge_file_cb(buff);

	Dbg("read_thread done\n");

	return NULL;
}
#endif

/* Warning: keeps the fd open. */
int breadhuge(struct buff *buff, const char *fname)
{
	int fd, len, pages, i;
	struct stat sbuf;
	struct page *page;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	if (fstat(fd, &sbuf)) {
		close(fd);
		return EIO;
	}

	bempty(buff);

	/* always read the first page */
	len = read(fd, buff->curcptr, PSIZE);
	if (len <= 0) {
		close(fd);
		return EIO;
	}
	buff->curpage->plen = len;
	buff->fd = fd;
	buff->size = sbuf.st_size;

	pages = (sbuf.st_size + PSIZE - 1) / PSIZE;
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

	do_lock();
	for (page = buff->firstp; page; page = page->nextp)
		page->pgoffset = 0;

	if (buff->fd >= 0) {
		close(buff->fd);
		buff->fd = -1;
	}
	do_unlock();
}
#endif
