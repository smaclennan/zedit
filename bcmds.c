/* bcmds.c - buffer oriented commands
 * Copyright (C) 1988-2013 Sean MacLennan
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

char Lbufname[BUFNAMMAX + 1];

static char **Bnames;			/* array of ptrs to buffer names */
static int Numbnames;			/* number of buffers */
static int Maxbnames;			/* max buffers Bnames can hold */

struct zbuff *Bufflist;		/* the buffer list */
struct zbuff *Curbuff;		/* the current buffer */
struct buff *Bbuff;         /* the current low-level buffer */

static void switchto_part(void)
{
	char word[STRMAX + 1];
	struct zbuff *tbuff;

	getbtxt(word, STRMAX);
	tbuff = cfindbuff(word);
	if (!tbuff)
		tbuff = Curbuff;
	if (nextbuff(tbuff))
		tbuff = nextbuff(tbuff);
	else
		tbuff = Bufflist;
	makepaw(zapp(tbuff)->bname, true);
}

void Zswitch_to_buffer(void)
{
	int rc;
	char *was = zapp(Curbuff)->bname;

	Arg = 0;
	Nextpart = switchto_part;
	rc = getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbnames);
	Nextpart = NULL;
	if (rc == -1)
		return;
	strcpy(Lbufname, was);
	uncomment(Curbuff);
	cswitchto(cfindbuff(Bnames[rc]));
}

void Znext_buffer(void)
{
	struct zbuff *next = prevbuff(Curbuff);


	if (!next && nextbuff(Curbuff))
		for (next = nextbuff(Curbuff); nextbuff(next); next = nextbuff(next))
			;
	if (next) {
		strcpy(Lbufname, zapp(Curbuff)->bname);
		uncomment(Curbuff);
		cswitchto(next);
		reframe();
	} else
		tbell();
}

static void delbuff(struct zbuff *buff)
{
	int wascur;
	struct wdo *wdo;

	wascur = buff == Curbuff;
	if (strcmp(Lbufname, zapp(buff)->bname) == 0)
		*Lbufname = '\0';
	if (cdelbuff(buff)) {
		if (wascur && *Lbufname) {
			struct zbuff *tbuff = cfindbuff(Lbufname);
			if (tbuff)
				zswitchto(tbuff);
		}
		cswitchto(Curbuff);

		/* make sure all windows pointed to deleted buff are updated */
		foreachwdo(wdo)
			if (wdo->wbuff == buff) {
				wdo->wbuff = Curbuff;
				bmrktopnt(Bbuff, wdo->wpnt);
				if (UMARK_SET) {
					mrktomrk(wdo->wmrk, UMARK);
					wdo->umark_set = 1;
				} else
					wdo->umark_set = 0;
				bmrktopnt(Bbuff, wdo->wstart);
				wdo->modeflags = INVALID;
			}
	} else
		error("Last Buffer.");
}

void Zdelete_buffer(void)
{
	struct zbuff *tbuff;
	char bname[BUFNAMMAX + 1];

	if (Argp) {
		strcpy(bname, Lbufname);
		do
			if (getarg("Buffer: ", bname, BUFNAMMAX))
				return;
		while ((tbuff = cfindbuff(bname)) == NULL);
		zswitchto(tbuff);
	}
	if (Bbuff->bmodf)
		switch (ask("save Changes? ")) {
		case ABORT:
			return;
		case YES:
			Zsave_file();
		}
	delbuff(Curbuff);
}

#define WASTED		(BUFNAMMAX + 14)

static void lstbuff(struct zbuff *tbuff)
{
	sprintf(PawStr, "%-*s %c%c %8lu %s\n", BUFNAMMAX, zapp(tbuff)->bname,
			(zapp(tbuff)->bmode & SYSBUFF) ? 'S' : ' ',
			tbuff->buff->bmodf ? '*' : ' ',
			blength(tbuff->buff),
			zapp(tbuff)->fname ? limit(zapp(tbuff)->fname, WASTED) : UNTITLED);
	binstr(Bbuff, PawStr);
}

void Zlist_buffers(void)
{
	struct wdo *was = Curwdo;
	int i;

	if (wuseother(LISTBUFF)) {
		for (i = 0; i < Numbnames; ++i) {
			struct zbuff *tbuff = cfindbuff(Bnames[i]);
			if (tbuff)
				lstbuff(tbuff);
			else {
				sprintf(PawStr, "%-*s Problem\n",
					BUFNAMMAX, Bnames[i]);
				binstr(Bbuff, PawStr);
			}
		}
		Bbuff->bmodf = false;
		wswitchto(was);
	} else
		tbell();
	Arg = 0;
}

void Zunmodify(void)
{
	Bbuff->bmodf = Argp;
}

/* Add the new bname to the Bname array.
 * If we hit Maxbnames, try to enlarge the Bnames array.
 * Note that the compare MUST be insensitive for the getplete!
 */
static char *addbname(const char *bname)
{
	int i;

	if (Numbnames == Maxbnames) {
		/* increase Bnames array */
		char **ptr = (char **)realloc(Bnames,
						  (Maxbnames + 10) * sizeof(char *));
		if (!ptr)
			return NULL;

		Bnames = ptr;
		Maxbnames += 10;
	}

	for (i = Numbnames; i > 0 && strcasecmp(bname, Bnames[i - 1]) < 0; --i)
		Bnames[i] = Bnames[i - 1];
	if ((Bnames[i] = strdup(bname))) {
		if (strlen(Bnames[i]) > BUFNAMMAX)
			Bnames[i][BUFNAMMAX] = '\0';
		++Numbnames;
	}

	return Bnames[i];
}

static bool delbname(char *bname)
{
	int i;

	for (i = 0; i < Numbnames && strcmp(bname, Bnames[i]); ++i)
		;
	if (i == Numbnames)
		return false;

	--Numbnames;
	free(bname);
	Bnames[i] = NULL;

	if (Numbnames == 0) {
		free(Bnames);
		Bnames = NULL;
		Maxbnames = 0;
	} else
		for (; i < Numbnames; ++i)
			Bnames[i] = Bnames[i + 1];

	return true;
}

static void bfini(void)
{
	Curbuff = NULL;

	while (Bufflist)
		/* bdelbuff will update Bufflist */
		cdelbuff(Bufflist);
}

static void binit(void)
{
	static int binitialized = 0;

	if (!binitialized) {
		bsetmod = vsetmod_callback;
		atexit(bfini);
		binitialized = 1;
	}
}

/* Create a buffer and add to Bufflist. */
struct zbuff *cmakebuff(const char *bname, char *fname)
{
	struct zbuff *bptr;

	if ((bptr = cfindbuff(bname))) {
		zswitchto(bptr);
		return bptr;
	}

	binit();

	if (!(bptr = calloc(1, sizeof(struct zbuff))) ||
		!(bptr->buff = bcreate()) ||
		!(bptr->bname = addbname(bname)) ||
		(fname && !(zapp(bptr)->fname = strdup(fname)))) {
		bdelbuff(bptr->buff);
		if (bptr->fname) free(bptr->fname);
		if (bptr->bname) delbname(bptr->bname);
		if (bptr) free(bptr);
		error("Unable to create buffer");
		return NULL;
	}

	/* add the buffer to the head of the list */
	if (Bufflist)
		prevbuff(Bufflist) = bptr;
	nextbuff(bptr) = Bufflist;
	Bufflist = bptr;

	bptr->buff->parent = bptr;

	zapp(bptr)->bmode = (VAR(VNORMAL) ? NORMAL : TXTMODE) |
		(VAR(VEXACT) ? EXACT : 0);

	if (*bname == '*')
		zapp(bptr)->bmode |= SYSBUFF;

	zswitchto(bptr);
	return bptr;
}

void zswitchto(struct zbuff *buf)
{
	if (buf && buf != Curbuff) {
		Curbuff = buf;
		Bbuff = buf->buff;
		makecur(Bbuff, buf->buff->curpage, buf->buff->curchar);
	}
}

/* WARNING: Curbuff could be set to NULL */
void bswitchto(struct buff *buf)
{
	if (buf && buf != Bbuff) {
		Curbuff = buf->parent;
		Bbuff = buf      ;
		makecur(Bbuff, buf->curpage, buf->curchar);
	}
}

bool cdelbuff(struct zbuff *tbuff)
{
	if (!tbuff)
		return false;

	if (zapp(tbuff)->bname) {
		delbname(zapp(tbuff)->bname);
		zapp(tbuff)->bname = NULL;
	}

	if (unvoke(tbuff))
		checkpipes(1);

	if (tbuff == Curbuff) { /* switch to a safe buffer */
		CLEAR_UMARK;

		if (nextbuff(tbuff))
			zswitchto(nextbuff(tbuff));
		else if (prevbuff(tbuff))
			zswitchto(prevbuff(tbuff));
		else
			return false;
	}

	if (tbuff == Bufflist)
		Bufflist = nextbuff(tbuff);
	if (prevbuff(tbuff))
		nextbuff(prevbuff(tbuff)) = nextbuff(tbuff);
	if (nextbuff(tbuff))
		prevbuff(nextbuff(tbuff)) = prevbuff(tbuff);

	if (zapp(tbuff)->fname)
		free(zapp(tbuff)->fname);
	uncomment(tbuff);
	undo_clear(tbuff);

	bdelbuff(tbuff->buff);
	free(tbuff);

	return true;
}

/* Locate a given buffer */
struct zbuff *cfindbuff(const char *bname)
{
	struct zbuff *tbuff;

	foreachbuff(tbuff)
		if (strncasecmp(zapp(tbuff)->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
}
