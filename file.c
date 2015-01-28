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

static bool cp(char *from, char *to)
{
	FILE *in, *out;
	char buf[1024];
	int rc = true;
	size_t n;

	in = fopen(from, "r");
	out = fopen(to, "w");
	if (!in || !out) {
		if (!in)
			fclose(in);
		return false;
	}
	while ((n = fread(buf, 1, 1024, in)) > 0)
		if (fwrite(buf, 1, n, out) != n) {
			rc = false;
			break;
		}
	fclose(in);
	fclose(out);
	return rc;
}

static char *make_bakname(char *bakname, char *fname)
{
	strcpy(bakname, fname);
	strcat(bakname, "~");
	return bakname;
}

bool zwritefile(char *fname)
{
	char bakname[PATHMAX + 1];
	struct stat sbuf;
	int nlink = 1, rc;
	bool bak = false;

	/* If the file existed, check to see if it has been modified. */
	if (Curbuff->mtime && stat(fname, &sbuf) == 0) {
		if (sbuf.st_mtime > Curbuff->mtime) {
			sprintf(PawStr,
					"WARNING: %s has been modified. Overwrite? ",
					lastpart(fname));
			if (ask(PawStr) != YES)
				return ABORT;
		}
		nlink = sbuf.st_nlink;
	}

	/* check for links and handle backup file */
	make_bakname(bakname, fname);
	if (nlink > 1) {
		sprintf(PawStr, "WARNING: %s is linked. Preserve? ",
			lastpart(fname));
		switch (ask(PawStr)) {
		case YES:
			if (VAR(VBACKUP))
				bak = cp(fname, bakname);
			break;
		case NO:
			if (VAR(VBACKUP))
				bak = rename(fname, bakname);
			else
				unlink(fname);	/* break link */
			break;
		case ABORT:
			return ABORT;
		}
	} else if (VAR(VBACKUP))
		bak = rename(fname, bakname);

	rc = bwritefile(fname);
	if (rc)
		clrpaw();
	else {
		if (errno == EACCES)
			error("File is read only.");
		else
			error("Unable to write file.");
		if (bak) {
			if (sbuf.st_nlink) {
				cp(bakname, fname);
				unlink(bakname);
			} else
				rename(bakname, fname);
		}
	}

	return rc;
}

/*
Load the file 'fname' into the current buffer.
Returns  0  successfully opened file
	 1  no such file
	-1  on error
*/
int zreadfile(char *fname)
{
	int rc = breadfile(fname);

	if (rc > 0) {
		if (rc == ENOENT)
			return 1;
		error("%s: %s", fname, strerror(errno));
		return -1;
	}

	if (rc == -1) {
		error("gzdopen %s", fname);
		return -1;
	}

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

	if (Curbuff->bmodf && ask("File modifed. Ok to loose changes?") != YES)
		return;

	/* Lose the undo history */
	undo_clear(Curbuff);

	offset = blocation();
	zreadfile(Curbuff->fname);
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
		int rc = zreadfile(path);
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
		rc = zreadfile(Fname);
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
