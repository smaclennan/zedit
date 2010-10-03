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
	rc = getplete("Buffer: ", Lbufname, Bnames, sizeof(char *), Numbuffs);
	Nextpart = ZNOTIMPL;
	if (rc == -1)
		return;
	strcpy(Lbufname, was);
	cswitchto(cfindbuff(Bnames[rc]));
}

void Znextbuff(void)
{
	struct buff *next = Curbuff->prev;


	if (!next && Curbuff->next)
		for (next = Curbuff->next; next->next; next = next->next)
			;
	if (next) {
		strcpy(Lbufname, Curbuff->bname);
		cswitchto(next);
		reframe();
	} else
		tbell();
}

void Zkillbuff(void)
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
			Zfilesave();
		}
	delbuff(Curbuff);
}


void delbuff(struct buff *buff)
{
	char bname[BUFNAMMAX + 1];
	int wascur;
	struct wdo *wdo;

	wascur = buff == Curbuff;
	strcpy(bname, buff->bname);	/* save it for delbname */
	if (strcmp(Lbufname, bname) == 0)
		*Lbufname = '\0';
	if (bdelbuff(buff)) {
#ifdef XWINDOWS
		XDeleteBuffer(bname);
#endif
		delbname(bname);
		if (wascur && *Lbufname) {
			struct buff *tbuff = cfindbuff(Lbufname);
			if (tbuff)
				bswitchto(tbuff);
		}
		cswitchto(Curbuff);

		/* make sure all windows pointed to deleted buff are updated */
		for (wdo = Whead; wdo; wdo = wdo->next)
			if (wdo->wbuff == buff) {
				wdo->wbuff = Curbuff;
				bmrktopnt(wdo->wpnt);
				Mrktomrk(wdo->wmrk, Curbuff->mark);
				bmrktopnt(wdo->wstart);
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
		blength(tbuff),
		tbuff->fname ? limit(tbuff->fname, WASTED) : UNTITLED);
	binstr(PawStr);
	binsert('\n');
}

void Zlstbuff(void)
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
		Curbuff->bmodf = FALSE;
		wswitchto(was);
	} else
		tbell();
	Arg = 0;
}

void Zunmodf(void)
{
	Curbuff->bmodf = Argp;
}
