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
static int Numbuffs;			/* number of buffers */
static int maxbuffs;			/* max buffers Bnames can hold */

struct buff *Bufflist;		/* the buffer list */

static void switchto_part(void)
{
	char word[STRMAX + 1];
	struct buff *tbuff;

	getbtxt(word, STRMAX);
	tbuff = cfindbuff(word);
	if (!tbuff)
		tbuff = Curbuff;
	if (tbuff->next)
		tbuff = tbuff->next;
	else
		tbuff = Bufflist;
	makepaw(tbuff->bname, true);
}

void Zswitch_to_buffer(void)
{
	int rc;
	char *was = Curbuff->bname;

	Arg = 0;
	Nextpart = switchto_part;
	rc = getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbuffs);
	Nextpart = NULL;
	if (rc == -1)
		return;
	strcpy(Lbufname, was);
	uncomment(Curbuff);
	cswitchto(cfindbuff(Bnames[rc]));
}

void Znext_buffer(void)
{
	struct buff *next = Curbuff->prev;


	if (!next && Curbuff->next)
		for (next = Curbuff->next; next->next; next = next->next)
			;
	if (next) {
		strcpy(Lbufname, Curbuff->bname);
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
	if (strcmp(Lbufname, buff->bname) == 0)
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
				bmrktopnt(wdo->wpnt);
				if (UMARK_SET) {
					mrktomrk(wdo->wmrk, UMARK);
					wdo->umark_set = 1;
				} else
					wdo->umark_set = 0;
				bmrktopnt(wdo->wstart);
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
	sprintf(PawStr, "%-*s %c%c %8lu %s ", BUFNAMMAX, tbuff->bname,
			(tbuff->bmode & SYSBUFF) ? 'S' : ' ',
			tbuff->bmodf ? '*' : ' ',
			blength(tbuff),
			zapp(tbuff)->fname ? limit(zapp(tbuff)->fname, WASTED) : UNTITLED);
	binstr(PawStr);
	binsert('\n');
}

void Zlist_buffers(void)
{
	struct wdo *was = Curwdo;
	int i;

	if (wuseother(LISTBUFF)) {
		for (i = 0; i < Numbuffs; ++i) {
			struct buff *tbuff = cfindbuff(Bnames[i]);
			if (tbuff)
				lstbuff(tbuff);
			else {
				sprintf(PawStr, "%-*s Problem\n",
					BUFNAMMAX, Bnames[i]);
				binstr(PawStr);
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

static void cfini(void)
{
	free(Bnames);
}

/* Add the new bname to the Bname array.
 * If we hit maxbuffs, try to enlarge the Bnames array.
 * Note that the compare MUST be insensitive for the getplete!
 */
static char *addbname(const char *bname)
{
	int i;

	if (Numbuffs == maxbuffs) {
		/* increase Bnames array */
		char **ptr = (char **)realloc(Bnames,
					      (maxbuffs + 10) * sizeof(char *));
		if (!ptr)
			return NULL;

		if (Bnames == NULL)
			atexit(cfini);

		Bnames = ptr;
		maxbuffs += 10;
	}

	for (i = Numbuffs; i > 0 && strcasecmp(bname, Bnames[i - 1]) < 0; --i)
		Bnames[i] = Bnames[i - 1];
	Bnames[i] = strdup(bname);
	if (strlen(Bnames[i]) > BUFNAMMAX)
		Bnames[i][BUFNAMMAX] = '\0';
	++Numbuffs;

	return Bnames[i];
}

/* Only fixes up the array - no frees */
static bool delbname(char *bname)
{
	int i;

	for (i = 0; i < Numbuffs && strcmp(bname, Bnames[i]); ++i)
		;
	if (i == Numbuffs)
		return false;

	--Numbuffs;
	Bnames[i] = NULL;

	for (; i < Numbuffs; ++i)
		Bnames[i] = Bnames[i + 1];

	return true;
}

static void zapp_cleanup(struct buff *bptr)
{
	if (bptr->app) {
		if (zapp(bptr)->fname)
			free(zapp(bptr)->fname);
		uncomment(bptr);
		undo_clear(bptr);
		free(bptr->app);
		bptr->app = NULL; /* paranoia */
	}
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
		app_cleanup = zapp_cleanup;
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

	if (!(bptr = _bcreate()) ||
		!(bptr->app = calloc(1, sizeof(struct zapp))) ||
		(fname && !(zapp(bptr)->fname = strdup(fname)))) {
		_bdelbuff(bptr);
		error("Unable to create buffer");
		return NULL;
	}

	bptr->bname = addbname(bname);
	if (!bptr->bname) {
		error("Out of buffers");
		_bdelbuff(bptr);
		return NULL;
	}

	/* add the buffer to the head of the list */
	if (Bufflist)
		Bufflist->prev = bptr;
	bptr->next = Bufflist;
	Bufflist = bptr;

	bptr->bmode = (VAR(VNORMAL) ? NORMAL : TXTMODE) |
		(VAR(VEXACT) ? EXACT : 0);

	if (*bname == '*')
		bptr->bmode |= SYSBUFF;

	bswitchto(bptr);
	return bptr;
}

bool cdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return false;

	if (tbuff->bname)
		delbname(tbuff->bname);

	if (unvoke(tbuff))
		checkpipes(1);

	CLEAR_UMARK;

	if (tbuff == Curbuff) { /* switch to a safe buffer */
		if (tbuff->next)
			bswitchto(tbuff->next);
		else if (tbuff->prev)
			bswitchto(tbuff->prev);
		else
			return false;
	}

	if (tbuff == Bufflist)
		Bufflist = tbuff->next;
	if (tbuff->prev)
		tbuff->prev->next = tbuff->next;
	if (tbuff->next)
		tbuff->next->prev = tbuff->prev;

	_bdelbuff(tbuff);

	return true;
}

/* Locate a given buffer */
struct buff *cfindbuff(const char *bname)
{
	struct buff *tbuff;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (strncasecmp(tbuff->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
}
