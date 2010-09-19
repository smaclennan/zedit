/* bcmds.c - buffer oriented commands
 * Copyright (C) 1988-2010 Sean MacLennan
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
#include <sys/stat.h>

void Zswitchto(void)
{
	char *was;
	int rc;

	Arg = 0;
	was = Curbuff->bname;
	Nextpart = ZSWITCHTO;
	rc = Getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbuffs);
	Nextpart = ZNOTIMPL;
	if (rc == -1)
		return;
	strcpy(Lbufname, was);
	Loadwdo(Bnames[rc]);
	Cswitchto(Cfindbuff(Bnames[rc]));
}

void Znextbuff(void)
{
	struct buff *next = Curbuff->prev;


	if (!next && Curbuff->next)
		for (next = Curbuff->next; next->next; next = next->next)
			;
	if (next) {
		strcpy(Lbufname, Curbuff->bname);
		Cswitchto(next);
		Reframe();
	} else
		Tbell();
}

void Zkillbuff(void)
{
	struct buff *tbuff;
	char bname[BUFNAMMAX + 1];

	if (Argp) {
		strcpy(bname, Lbufname);
		do
			if (Getarg("Buffer: ", bname, BUFNAMMAX))
				return;
		while ((tbuff = Cfindbuff(bname)) == NULL);
		Bswitchto(tbuff);
	}
	if (Curbuff->bmodf)
		switch (Ask("Save Changes? ")) {
		case ABORT:
			return;
		case YES:
			Zfilesave();
		}
	Delbuff(Curbuff);
}


void Delbuff(struct buff *buff)
{
	char bname[BUFNAMMAX + 1];
	int wascur;
	struct wdo *wdo;

	wascur = buff == Curbuff;
	strcpy(bname, buff->bname);	/* save it for Delbname */
	if (strcmp(Lbufname, bname) == 0)
		*Lbufname = '\0';
	if (Bdelbuff(buff)) {
#if XWINDOWS
		XDeleteBuffer(bname);
#endif
		Delbname(bname);
		if (wascur && *Lbufname) {
			struct buff *tbuff = Cfindbuff(Lbufname);
			if (tbuff)
				Bswitchto(tbuff);
		}
		Cswitchto(Curbuff);

		/* make sure all windows pointed to deleted buff are updated */
		for (wdo = Whead; wdo; wdo = wdo->next)
			if (wdo->wbuff == buff) {
				wdo->wbuff = Curbuff;
				Bmrktopnt(wdo->wpnt);
				Mrktomrk(wdo->wmrk, Curbuff->mark);
				Bmrktopnt(wdo->wstart);
				wdo->modeflags = INVALID;
			}
	}
}

#define WASTED		(BUFNAMMAX + 14)

static void lstbuff(struct buff *tbuff)
{
	sprintf(PawStr, "%-*s %c%c %8lu %s ", BUFNAMMAX, tbuff->bname,
		(tbuff->bmode & SYSBUFF) ? 'S' : ' ',
		tbuff->bmodf ? '*' : ' ',
		Blength(tbuff),
		tbuff->fname ? Limit(tbuff->fname, WASTED) : UNTITLED);
	Binstr(PawStr);
	Binsert('\n');
}

void Zlstbuff(void)
{
	struct wdo *was = Curwdo;
	int i;

	if (WuseOther(LISTBUFF)) {
		for (i = 0; i < Numbuffs; ++i) {
			struct buff *tbuff = Cfindbuff(Bnames[i]);
			if (tbuff)
				lstbuff(tbuff);
			else {
				sprintf(PawStr, "%-*s Problem\n",
					BUFNAMMAX, Bnames[i]);
				Binstr(PawStr);
			}
		}
		Curbuff->bmodf = FALSE;
		Wswitchto(was);
	} else
		Tbell();
	Arg = 0;
}

void Zunmodf(void)
{
	Curbuff->bmodf = FALSE;
}
