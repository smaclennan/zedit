/* bcmds.c - buffer oriented commands
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

static char Lbufname[BUFNAMMAX + 1];

static char **Bnames;			/* array of ptrs to buffer names */
static int Numbnames;			/* number of buffers */
static int Maxbnames;			/* max buffers Bnames can hold */

zbuff_t *Bufflist;		/* the buffer list */
static zbuff_t *Bufflist_tail;
zbuff_t *Curbuff;		/* the current buffer */
struct buff *Bbuff;         /* the current low-level buffer */

/* Set Lbufname */
void set_last_bufname(zbuff_t *buff)
{
	strlcpy(Lbufname, buff->bname, sizeof(Lbufname));
}

/* Get the word at the current buffer point and store in 'word'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
int getbword(char word[], int max, int (*valid)(int))
{
	int i;
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	if (!bistoken(Buff()))
		bmoveto(Bbuff, bistoken, BACKWARD);
	bmovepast(Bbuff, bistoken, BACKWARD);
	for (i = 0; !bisend(Bbuff) && valid(Buff()) && i < max; ++i, bmove1(Bbuff))
		word[i] = Buff();
	word[i] = '\0';
	bpnttomrk(Bbuff, &tmark);
	return i;
}

/* Get the current buffer text and store in 'txt'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
char *getbtxt(char txt[], int max)
{
	int i;
	struct mark tmark;

	bmrktopnt(Bbuff, &tmark);
	for (btostart(Bbuff), i = 0; !bisend(Bbuff) && i < max; bmove1(Bbuff), ++i)
		txt[i] = Buff();
	txt[i] = '\0';
	bpnttomrk(Bbuff, &tmark);
	return txt;
}

static void switchto_part(void)
{
	char word[STRMAX + 1];
	zbuff_t *tbuff;

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
	zbuff_t *was = Curbuff;

	Arg = 0;
	Nextpart = switchto_part;
	rc = getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbnames);
	Nextpart = NULL;
	if (rc == -1)
		return;
	set_last_bufname(was);
	uncomment(Curbuff);
	cswitchto(cfindbuff(Bnames[rc]));
}

void Znext_buffer(void)
{
	zbuff_t *next = Curbuff->next;

	if (!next)
		next = Bufflist;
	if (next) {
		set_last_bufname(Curbuff);
		uncomment(Curbuff);
		cswitchto(next);
		reframe();
	} else
		tbell();
}

static void delbuff(zbuff_t *buff)
{
	struct wdo *wdo;

	if (strcmp(Lbufname, buff->bname) == 0)
		*Lbufname = '\0';
	if (cdelbuff(buff)) {
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
	zbuff_t *tbuff;
	char bname[BUFNAMMAX + 1];

	if (Argp) {
		strlcpy(bname, Lbufname, sizeof(bname));
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

/* Limit a filename to at most Colmax - 'num' cols */
char *limit(char *fname, int num)
{
	int off = strlen(fname) - (Colmax - num);
	return off > 0 ? fname + off : fname;
}

static void lstbuff(zbuff_t *tbuff)
{
	binstr(Bbuff, "%-16s %s%s %8u %s\n", tbuff->bname,
		   (tbuff->bmode & SYSBUFF) ? "S" : " ",
		   tbuff->buff->bmodf ? "*" : " ",
		   blength(tbuff->buff),
		   tbuff->fname ? limit(tbuff->fname, WASTED) : UNTITLED);
}

void Zlist_buffers(void)
{
	struct wdo *was = Curwdo;
	int i;

	if (wuseother(LISTBUFF)) {
		for (i = 0; i < Numbnames; ++i) {
			zbuff_t *tbuff = cfindbuff(Bnames[i]);
			if (tbuff)
				lstbuff(tbuff);
			else if (strcmp(Bnames[i], "*paw*"))
				binstr(Bbuff, "%-16s Problem\n", Bnames[i]);
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

	cdelbuff(Paw);

	delfini();
	mrkfini(); /* must be last */

	Dbgfname(NULL);
}

static void binit(void)
{
	static int binitialized = 0;

	if (!binitialized) {
		bsetmod = vsetmod_callback;
		delinit();
		atexit(bfini);
		binitialized = 1;
	}
}

/* Create a buffer and add to Bufflist. Do not switch to buffer. */
zbuff_t *zcreatebuff(const char *bname, char *fname)
{
	zbuff_t *bptr;

	bptr = cfindbuff(bname);
	if (bptr)
		return bptr;

	binit();

	if (!(bptr = calloc(1, sizeof(zbuff_t))) ||
		!(bptr->buff = bcreate()) ||
		(fname && !(bptr->fname = strdup(fname)))) {
		bdelbuff(bptr->buff);
		free(bptr->fname);
		free(bptr);
		error("Unable to create buffer");
		return NULL;
	}

	if (bname && !(bptr->bname = addbname(bname))) {
		error("Out of buffer name space");
		cdelbuff(bptr);
		return NULL;
	}

	/* add the buffer to the tail of the list */
	if (Bufflist) {
		Bufflist_tail->next = bptr;
		bptr->prev = Bufflist_tail;
	} else
		Bufflist = bptr;
	Bufflist_tail = bptr;

	bptr->bmode = (VAR(VNORMAL) ? NORMAL : TXTMODE) |
		(VAR(VEXACT) ? EXACT : 0);

	if (bname && *bname == '*') {
		bptr->bmode |= SYSBUFF;
#if UNDO
		/* this disables undo for system buffers */
		bptr->buff->in_undo = 1;
#endif
	}

	return bptr;
}

/* Create a buffer and add to Bufflist. Switch to buffer. */
zbuff_t *cmakebuff(const char *bname, char *fname)
{
	zbuff_t *bptr = zcreatebuff(bname, fname);
	zswitchto(bptr);
	return bptr;
}

void zswitchto(zbuff_t *buf)
{
	if (buf && buf != Curbuff) {
		Curbuff = buf;
		Bbuff = buf->buff;

		/* This makes the new buffs page current. */
		struct mark tmp;
		bmrktopnt(Bbuff, &tmp);
		bpnttomrk(Bbuff, &tmp);
	}
}

bool cdelbuff(zbuff_t *tbuff)
{
	if (!tbuff)
		return false;

	 /* Make sure we can switch to a safe buffer before tearing
	  * anything down.
	  */
	if (tbuff == Curbuff) {
		CLEAR_UMARK;

		if (tbuff->next)
			zswitchto(tbuff->next);
		else if (tbuff->prev)
			zswitchto(tbuff->prev);
		else
			return false;
	}

	/* Do this before deleting bname */
	undo_clear(tbuff->buff);
	uncomment(tbuff);

	if (tbuff->bname) {
		delbname(tbuff->bname);
		tbuff->bname = NULL;
	}

	if (unvoke(tbuff))
		checkpipes(1);

	if (tbuff == Bufflist)
		Bufflist = tbuff->next;
	if (tbuff == Bufflist_tail)
		Bufflist_tail = tbuff->prev;
	if (tbuff->prev)
		tbuff->prev->next = tbuff->next;
	if (tbuff->next)
		tbuff->next->prev = tbuff->prev;

	free(tbuff->fname);

	bdelbuff(tbuff->buff);
	free(tbuff);

	return true;
}

/* Locate a given buffer */
zbuff_t *cfindbuff(const char *bname)
{
	zbuff_t *tbuff;

	foreachbuff(tbuff)
		if (strncasecmp(tbuff->bname, bname, BUFNAMMAX) == 0)
			return tbuff;
	return NULL;
}

zbuff_t *cfindzbuff(struct buff *buff)
{
	zbuff_t *tbuff;

	foreachbuff(tbuff)
		if (tbuff->buff == buff)
			return tbuff;
	return NULL;
}
