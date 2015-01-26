/* file.c - Zedit file commands
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

static char Fname[PATHMAX + 1];

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

	if (Curbuff->bmodf && ask("File modifed. Ok to loose changes?") != YES)
		return;

	/* Lose the undo history */
	undo_clear(Curbuff);

	offset = blocation();
	breadfile(Curbuff->fname);
	boffset(offset);
	uncomment(Curbuff);
}

/* Read one file, creating the buffer if necessary.
 * Returns false if unable to create buffer only.
 * false means that there is no use continuing.
 */
static bool readone(char *bname, char *path)
{
	struct buff *was = Curbuff;

	if (cfindbuff(bname))
		return true;

	if (cmakebuff(bname, path)) {
		putpaw("Reading %s", lastpart(path));
		int rc = breadfile(path);
		if (rc >= 0) {
			toggle_mode(0);
			if (rc > 0)
				putpaw("New File");
#ifdef R_OK
			else if (access(path, R_OK | W_OK) == EOF)
				Curbuff->bmode |= VIEW;
#endif
			strcpy(Lbufname, was->bname);
		} else { /* error */
			delbname(Curbuff->bname);
			bdelbuff(Curbuff);
			bswitchto(was);
		}
		return true;
	}
	return false;
}


bool findfile(char *path)
{
	char tbname[BUFNAMMAX + 1];
	char *was;
	struct buff *tbuff;
	int rc = true;

	Arg = 0;
	was = Curbuff->bname;

	/* limit name to BUFNAMMAX */
	strncpy(tbname, lastpart(path), BUFNAMMAX);
	tbname[BUFNAMMAX] = '\0';

	/* If is this file already in a buffer - use it.
	 * At startup, we are done.
	 */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->fname && strcmp(path, tbuff->fname) == 0) {
			bswitchto(tbuff);
			if (Initializing)
				return true;
			strcpy(Lbufname, was);
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
			i = 0;
			do
				sprintf(p, ".%d", ++i);
			while (cfindbuff(tbname));
		}

		rc = readone(tbname, path);
	}

	if (!Initializing) {
		cswitchto(Curbuff);
		reframe();
	} else if (!*Fname)
		strcpy(Fname, path);

	return rc;
}

void Zsave_all_files(void)
{
	if (Argp) {
		struct buff *tbuff;

		for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if (!(tbuff->bmode & SYSBUFF) && tbuff->fname)
				tbuff->bmodf = true;
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
		if (Argp) {
			struct buff *tbuff, *save = Curbuff;

			tbuff = bcreate();
			if (tbuff) {
				NEED_UMARK;
				putpaw("Writing %s", path);
				bswitchto(save);
				bcopyrgn(Curbuff->umark, tbuff);
				bswitchto(tbuff);
				Curbuff->bmode = save->bmode;
				zwritefile(path);
				bswitchto(save);
				bdelbuff(tbuff);
				clrpaw();
				CLEAR_UMARK;
			}
		} else {
			if (Curbuff->fname)
				free(Curbuff->fname);
			Curbuff->fname = strdup(path);
			Curbuff->mtime = 0;	/* this is no longer valid */
			Zsave_file();
			Curwdo->modeflags = INVALID;
		}
	}
}

void Zread_file(void)
{
	struct buff *tbuff, *save;
	struct mark *tmark;
	int rc = 1;

	if (get_findfile("Read File: "))
		return;

	save = Curbuff;
	tbuff = bcreate();
	if (tbuff) {
		bswitchto(tbuff);
		Curbuff->bmode = save->bmode;
		putpaw("Reading %s", lastpart(Fname));
		rc = breadfile(Fname);
		if (rc == 0) {
			btoend();
			tmark = zcreatemrk();
			btostart();
			bcopyrgn(tmark, save);
			unmark(tmark);
		}
		bswitchto(save);
		bdelbuff(tbuff);
	}

	if (rc > 0)
		error("Unable to read %s", Fname);
}
