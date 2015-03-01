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

struct buff *Bufflist;		/* the buffer list */
struct buff *Curbuff;		/* the current buffer */

static void switchto_part(void)
{
	char word[STRMAX + 1];
	struct buff *tbuff;

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
	struct buff *next = prevbuff(Curbuff);


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

static void delbuff(struct buff *buff)
{
	int wascur;
	struct wdo *wdo;

	wascur = buff == Curbuff;
	if (strcmp(Lbufname, zapp(buff)->bname) == 0)
		*Lbufname = '\0';
	if (cdelbuff(buff)) {
		if (wascur && *Lbufname) {
			struct buff *tbuff = cfindbuff(Lbufname);
			if (tbuff)
				bswitchto(tbuff);
		}
		cswitchto(Curbuff);

		/* make sure all windows pointed to deleted buff are updated */
		foreachwdo(wdo)
			if (wdo->wbuff == buff) {
				wdo->wbuff = Curbuff;
				bmrktopnt(Curbuff, wdo->wpnt);
				if (UMARK_SET) {
					mrktomrk(wdo->wmrk, UMARK);
					wdo->umark_set = 1;
				} else
					wdo->umark_set = 0;
				bmrktopnt(Curbuff, wdo->wstart);
				wdo->modeflags = INVALID;
			}
	} else
		error("Last Buffer.");
}

void Zdelete_buffer(void)
{
	struct buff *tbuff;
	char bname[BUFNAMMAX + 1];

	if (Argp) {
		strcpy(bname, Lbufname);
		do
			if (getarg("Buffer: ", bname, BUFNAMMAX))
				return;
		while ((tbuff = cfindbuff(bname)) == NULL);
		bswitchto(tbuff);
	}
	if (Curbuff->bmodf)
		switch (ask("save Changes? ")) {
		case ABORT:
			return;
		case YES:
			Zsave_file();
		}
	delbuff(Curbuff);
}

#define WASTED		(BUFNAMMAX + 14)

static void lstbuff(struct buff *tbuff)
{
	sprintf(PawStr, "%-*s %c%c %8lu %s\n", BUFNAMMAX, zapp(tbuff)->bname,
			(zapp(tbuff)->bmode & SYSBUFF) ? 'S' : ' ',
			tbuff->bmodf ? '*' : ' ',
			blength(tbuff),
			zapp(tbuff)->fname ? limit(zapp(tbuff)->fname, WASTED) : UNTITLED);
	binstr(Curbuff, PawStr);
}

void Zlist_buffers(void)
{
	struct wdo *was = Curwdo;
	int i;

	if (wuseother(LISTBUFF)) {
		for (i = 0; i < Numbnames; ++i) {
			struct buff *tbuff = cfindbuff(Bnames[i]);
			if (tbuff)
				lstbuff(tbuff);
			else {
				sprintf(PawStr, "%-*s Problem\n",
					BUFNAMMAX, Bnames[i]);
				binstr(Curbuff, PawStr);
			}
		}
		Curbuff->bmodf = false;
		wswitchto(was);
	} else
		tbell();
	Arg = 0;
}

void Zunmodify(void)
{
	Curbuff->bmodf = Argp;
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
struct buff *cmakebuff(const char *bname, char *fname)
{
	struct buff *bptr;

	if ((bptr = cfindbuff(bname))) {
		bswitchto(bptr);
		return bptr;
	}

	binit();

	if (!(bptr = bcreate()) ||
		!(bptr->app = calloc(1, sizeof(struct zapp))) ||
		(fname && !(zapp(bptr)->fname = strdup(fname)))) {
		bdelbuff(bptr);
		error("Unable to create buffer");
		return NULL;
	}

	zapp(bptr)->bname = addbname(bname);
	if (!zapp(bptr)->bname) {
		error("Out of buffers");
		bdelbuff(bptr);
		return NULL;
	}

	/* add the buffer to the head of the list */
	if (Bufflist)
		prevbuff(Bufflist) = bptr;
	nextbuff(bptr) = Bufflist;
	Bufflist = bptr;

	zapp(bptr)->bmode = (VAR(VNORMAL) ? NORMAL : TXTMODE) |
		(VAR(VEXACT) ? EXACT : 0);

	if (*bname == '*')
		zapp(bptr)->bmode |= SYSBUFF;

	bswitchto(bptr);
	return bptr;
}

void bswitchto(struct buff *buf)
{
	if (buf && buf != Curbuff) {
		Curbuff = buf;
		makecur(Curbuff, buf->curpage, buf->curchar);
	}
}

bool cdelbuff(struct buff *tbuff)
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
			bswitchto(nextbuff(tbuff));
		else if (prevbuff(tbuff))
			bswitchto(prevbuff(tbuff));
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
	free(tbuff->app);

	bdelbuff(tbuff);

	return true;
}

/* Locate a given buffer */
struct buff *cfindbuff(const char *bname)
{
	struct buff *tbuff;

	foreachbuff(tbuff)
		if (strncasecmp(zapp(tbuff)->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
}
