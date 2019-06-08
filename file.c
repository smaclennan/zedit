/* file.c - Zedit file commands
 * Copyright (C) 1988-2018 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "z.h"
#include <setjmp.h>

/** @addtogroup zedit
 * @{
*/

static char Fname[PATHMAX + 1];
int raw_mode;

/* Return a pointer to the start of the last part of fname */
char *lastpart(char *fname)
{
	char *p = strrchr(fname, '/');
	if (p)
		return p + 1;
	else
		return fname;
}

int file_mode(void)
{
	static int Cmask = 0;

	if (Cmask == 0) {
		Cmask = umask(0);	/* get the current umask */
		umask(Cmask);		/* set it back */
		Cmask = ~Cmask & 0666;	/* make it usable */
	}

	return Cmask | (Curbuff->bmode & (FILE_COMPRESSED | FILE_CRLF));
}

static bool zwritefile(char *fname)
{
	struct mark smark;
	struct stat sbuf;
	int nlink = 1, rc;

	/* If the file existed, check to see if it has been modified. */
	if (Curbuff->mtime && stat(fname, &sbuf) == 0) {
		if (sbuf.st_mtime > Curbuff->mtime) {
			strconcat(PawStr, PAWSTRLEN,
					  "WARNING: ", lastpart(fname),
					  " has been modified. Overwrite? ", NULL);
			if (ask(PawStr) != YES)
				return false;
		}
#ifndef __QNXNTO__
		nlink = sbuf.st_nlink;
#endif
	}

#if HUGE_FILES
	if (Bbuff->fd != -1) {
		strconcat(PawStr, PAWSTRLEN,
				  "WARNING: huge file ", lastpart(fname),
				  " not yet read. Are you sure? ", NULL);
		if (ask(PawStr) != YES)
			return false;
	}
#endif

	/* check for links */
	if (nlink > 1) {
		strconcat(PawStr, PAWSTRLEN, "WARNING: ", lastpart(fname),
				  " is linked. Preserve? ", NULL);
		switch (ask(PawStr)) {
		case YES:
			break;
		case NO:
			unlink(fname);	/* break link */
			break;
		case ABORT:
			return false;
		}
	}

	bmrktopnt(Bbuff, &smark);
	rc = bwritefile(Bbuff, fname, file_mode());
	bpnttomrk(Bbuff, &smark);
	if (rc) {
		if (stat(fname, &sbuf) == 0)
			Curbuff->mtime = sbuf.st_mtime;
		else
			Curbuff->mtime = -1;
		clrpaw();
	} else {
		if (errno == EACCES)
			error("File is read only.");
		else
			error("Unable to write file.");
	}

	return rc;
}

static void crfixup(void)
{
	char *p = (char *)memchr(Bbuff->curpage->pdata + 1, '\n', curplen(Bbuff) - 1);
	if (!p)
		return;

	if (*(p - 1) != '\r')
		return;

	if (raw_mode)
		return;

	Curbuff->bmode |= FILE_CRLF;

	while (bcsearch(Bbuff, '\r'))
		if (Buff() == '\n') {
			bmove(Bbuff, -1);
			bdelete(Bbuff, 1);
		}

	btostart(Bbuff);
}

/*
Load the file 'fname' into the current buffer.
Returns  0  successfully opened file
	 1  no such file
	-1  on error
*/
int zreadfile(char *fname)
{
	struct stat sbuf;
	int compressed, rc;

	if (stat(fname, &sbuf)) {
		if (errno == ENOENT)
			memset(&sbuf, 0, sizeof(sbuf));
		else
			return -1;
	}

	Curbuff->mtime = sbuf.st_mtime;

	bempty(Bbuff);

#if HUGE_FILES
	if (sbuf.st_size > HUGE_SIZE) {
		compressed = 0;
		rc = breadhuge(Bbuff, fname);
	} else
#endif
		rc = breadfile(Bbuff, fname, &compressed);

	if (rc < 0) {
		if (rc == -ENOENT)
			return 1;
		error("%s: %s", fname, strerror(-rc));
		return -1;
	}

	if (rc == 1) {
		error("gzdopen %s", fname);
		return -1;
	}

	if (compressed)
		Curbuff->bmode |= FILE_COMPRESSED;
	else if (curplen(Bbuff))
		crfixup();

	Bbuff->bmodf = false;

	clrpaw();
	return 0;
}

static int get_findfile(const char *prompt)
{
	struct stat sbuf;

	if (!*Fname)
		zgetcwd(Fname, PATHMAX);
	else if (stat(Fname, &sbuf) == 0)
		/* If Fname is a file, convert to directory */
		if (sbuf.st_mode & S_IFREG) {
			char *p = strrchr(Fname, '/');
			if (p)
				*(p + 1) = '\0';
		}

	return getfname(prompt, Fname);
}

void Zfind_file(void)
{
	if (get_findfile("Find File: ") == 0)
		if (findfile(Fname))
			uncomment(NULL);
}

void Zrevert_file(void)
{
	unsigned long offset;

	if (!Curbuff->fname) {
		tbell();
		return;
	}

	if (Bbuff->bmodf && ask("File modifed. Ok to loose changes?") != YES)
		return;

	/* Lose the undo history */
	undo_clear(Curbuff->buff);

	offset = blocation(Bbuff);
	zreadfile(Curbuff->fname);
	boffset(Bbuff, offset);
	uncomment(Curbuff);
}

/* Read one file, creating the buffer if necessary. */
static bool readone(char *bname, char *path)
{
	int rc;
	zbuff_t *was = Curbuff;

	if (cfindbuff(bname))
		return true;

	if (!cmakebuff(bname, path))
		return false;

	putpaw("Reading %s", lastpart(path));
	rc = zreadfile(path);
	if (rc < 0) {
		cdelbuff(Curbuff);
		zswitchto(was);
		putpaw("Read Error %s", lastpart(path));
		return false;
	}

	toggle_mode(0);
	if (rc > 0)
		putpaw("New File");
#ifdef R_OK
	else if (access(path, R_OK | W_OK) == EOF)
		Curbuff->bmode |= VIEW;
#endif
	set_last_bufname(was);
	return true;
}

bool findfile(char *path)
{
	char tbname[BUFNAMMAX + 1];
	zbuff_t *tbuff;
	zbuff_t *was = Curbuff;

	Arg = 0;

	/* limit name to BUFNAMMAX */
	strncpy(tbname, lastpart(path), BUFNAMMAX);
	tbname[BUFNAMMAX] = '\0';

	/* If is this file already in a buffer - use it.
	 * At startup, we are done.
	 */
	foreachbuff(tbuff)
		if (tbuff->fname && strcmp(path, tbuff->fname) == 0) {
			zswitchto(tbuff);
			if (Initializing)
				return true;
			set_last_bufname(was);
			break;
		}

	if (!tbuff) {
		if (cfindbuff(tbname)) {
			/* Resolve buffer name collisions by creating
			 * a unique name */
			char *p;
			int i;

			i = strlen(tbname);
			p = &tbname[i < BUFNAMMAX - 3 ? i : BUFNAMMAX - 3];
			/* We cannot use 100 here due to a bug in gcc 7.3.0 */
			for (i = 0; i < 99; ++i) {
				strfmt(p, 4, ".%d", i);
				if (!cfindbuff(tbname))
					break;
			}
			if (cfindbuff(tbname))
				return false;
		}

		if (!readone(tbname, path))
			return false;
	}

	if (!Initializing) {
		cswitchto(Curbuff);
		reframe();
	} else if (!*Fname)
		strcpy(Fname, path);

	return true;
}

void Zsave_all_files(void)
{
	if (Argp) {
		zbuff_t *tbuff;

		foreachbuff(tbuff)
			if (!(tbuff->bmode & SYSBUFF) && tbuff->fname)
				tbuff->buff->bmodf = true;
	}
	saveall(true);
}

void Zsave_file(void)
{
	if (Argp)
		saveall(false);
	else
		filesave();
}

bool filesave(void)
{
	char path[PATHMAX + 1];

	Arg = 0;
	if (Curbuff->fname == NULL) {
		*path = '\0';
		if (getfname("File Name: ", path) == 0)
			Curbuff->fname = strdup(path);
		else
			return false;
		Curwdo->modeflags = INVALID;
	}
	putpaw("Writing %s", lastpart(Curbuff->fname));
	return zwritefile(Curbuff->fname);
}

void Zwrite_file(void)
{
	char path[PATHMAX + 1];
	const char *prompt = Argp ? "Write Region: " : "Write File: ";

	Arg = 0;
	*path = '\0';
	if (getfname(prompt, path) == 0) {
		free(Curbuff->fname);
		Curbuff->fname = strdup(path);
		Curbuff->mtime = 0;	/* this is no longer valid */
		Zsave_file();
		Curwdo->modeflags = INVALID;
	}
}

void Zread_file(void)
{
	struct buff *tbuff;
	struct mark *tmark;
	int rc = 1;

	if (get_findfile("Read File: "))
		return;

	if ((tbuff = bcreate())) {
		putpaw("Reading %s", lastpart(Fname));
		rc = breadfile(tbuff, Fname, NULL);
		if (rc == 0) {
			btoend(tbuff);
			if ((tmark = bcremark(tbuff))) {
				btostart(tbuff);
				bcopyrgn(tmark, Bbuff);
				bdelmark(tmark);
			} else
				error("Out of memory");
		}
		bdelbuff(tbuff);
	}

	if (rc)
		error("Unable to read %s", Fname);
}
/* @} */
