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

static int get_findfile(char *prompt)
{
	struct stat sbuf;

	if (!*Fname)
		sprintf(Fname, "%s/", Cwd);
	else if (stat(Fname, &sbuf) == 0)
		/* If Fname is a file, convert to directory */
		if (S_ISREG(sbuf.st_mode)) {
			char *p = strrchr(Fname, '/');
			if (p)
				*(p + 1) = '\0';
		}

	return getfname(prompt, Fname);
}

/***
 * Prompts for a path name. If a buffer already exists with this path name,
 * that buffer is switched to. If no buffer is matched, a new buffer is
 * created and the file read into it. Supports file name completion. A
 * Universal Argument causes the command to repeat.
 */
void Zfind_file(void)
{
	if (get_findfile("Find File: ") == 0)
		findfile(Fname);
}

/***
 * Rereads the current file. If the file has changed, asks to overwrite
 * changes. A Universal Argument causes multiple rereads. Mimics the vi "e"
 * command.
 */
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

	offset = blocation(NULL);
	breadfile(Curbuff->fname);
	boffset(offset);
}

/* Read one file, creating the buffer is necessary.
 * Returns false if unable to create buffer only.
 * false means that there is no use continuing.
 */
static bool readone(char *bname, char *path)
{
	struct buff *was = Curbuff;

	if (cfindbuff(bname))
		return true;

	if (cmakebuff(bname, path)) {
		int rc = breadfile(path);
		if (rc >= 0) {
			toggle_mode(0);
			if (rc > 0)
				putpaw("New File");
			else if (access(path, R_OK|W_OK) == EOF)
				Curbuff->bmode |= VIEW;
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

/***
 * Saves all modified buffers. A Universal Argument causes ALL files to be
 * saved, modified or not.
 */
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

/***
 * Saves the current buffer to its file. If the current buffer has no path
 * name, a path name is prompted for (with file name completion). A
 * Universal Argument is ignored.
 */
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
	return bwritefile(Curbuff->fname);
}

/*
 * Write the region to 'path'. Assumes 'path' correct.
 * Returns: true, false, ABORT
 */
static int write_rgn(char *path)
{
	struct buff *tbuff, *save;
	int rc = false;

	save = Curbuff;
	tbuff = cmakebuff("___tmp___", (char *)NULL);
	if (tbuff) {
		bswitchto(save);
		bcopyrgn(Curbuff->mark, tbuff);
		bswitchto(tbuff);
		Curbuff->bmode = save->bmode;
		rc = bwritefile(path);
		bswitchto(save);
		bdelbuff(tbuff);
	}
	return rc;
}

/***
 * Prompts for a path name and writes out the current buffer to this file
 * name. Supports file name completion. It changes the path name of the
 * current buffer to the new path name. A Universal Argument causes only
 * the Region to be written and does not change the name of the buffer.
 */
void Zwrite_file(void)
{
	char path[PATHMAX + 1], *prompt;

	Arg = 0;
	prompt = Argp ? "Write Region: " : "Write File: ";
	*path = '\0';
	if (getfname(prompt, path) == 0) {
		if (Argp) {
			putpaw("Writing %s", path);
			write_rgn(path);
			clrpaw();
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

/* read 'fname' into buffer at Point */
static int fileread(char *fname)
{
	struct buff *tbuff, *save;
	struct mark *tmark;
	int rc = 1;

	save = Curbuff;
	tbuff = bcreate();
	if (tbuff) {
		bswitchto(tbuff);
		Curbuff->bmode = save->bmode;
		rc = breadfile(fname);
		if (rc == 0) {
			btoend();
			tmark = bcremrk();
			btostart();
			bcopyrgn(tmark, save);
			unmark(tmark);
		}
		bswitchto(save);
		bdelbuff(tbuff);
	}
	return rc;
}

/***
 * Prompts for a path name, with file name completion,  and inserts the
 * file into the current buffer before the Point. If there is a Universal
 * Argument, the current buffer is deleted, first asking to save the file
 * if it is modified, before reading in the new file. The buffers file name
 * is not changed, any writes will be to the old file name.
 */
void Zread_file(void)
{
	if (get_findfile("Read File: "))
		return;
	if (fileread(Fname) > 0)
		error("Unable to read %s", Fname);
}
