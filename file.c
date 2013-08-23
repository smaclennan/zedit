/* file.c - Zedit file commands
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
#include <assert.h>

static char Fname[PATHMAX + 1];

static struct passwd *zgetpwnam(char *name);

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

void Zfindfile(void)
{
	if (get_findfile("Find File: ") == 0)
		findfile(Fname, FALSE);
}

void Zrevertfile(void)
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

Boolean findfile(char *path, int startup)
{
	char tbname[BUFNAMMAX + 1];
	char *was;
	struct buff *tbuff;
	int rc = TRUE;

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
			if (startup)
				return TRUE;
			bswitchto(tbuff);
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

	if (!startup) {
		cswitchto(Curbuff);
		reframe();
	}

	return rc;
}

void Zsaveall(void)
{
	if (Argp) {
		struct buff *tbuff;

		for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if (!(tbuff->bmode & SYSBUFF) && tbuff->fname)
				tbuff->bmodf = MODIFIED;
	}
	saveall(TRUE);
}

void Zfilesave(void)
{
	if (Argp)
		saveall(FALSE);
	else
		filesave();
}

Boolean filesave(void)
{
	char path[PATHMAX + 1];

	Arg = 0;
	if (Curbuff->fname == NULL) {
		*path = '\0';
		if (getfname("File Name: ", path) == 0)
			Curbuff->fname = strdup(path);
		else
			return FALSE;
		Curwdo->modeflags = INVALID;
	}
	putpaw("Writing %s", lastpart(Curbuff->fname));
	return bwritefile(Curbuff->fname);
}

/*
 * Write the region to 'path'. Assumes 'path' correct.
 * Returns: TRUE, FALSE, ABORT
 */
static int write_rgn(char *path)
{
	struct buff *tbuff, *save;
	int rc = FALSE;

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

void Zfilewrite(void)
{
	char path[PATHMAX + 1], *prompt;

	Arg = 0;
	prompt = Argp ? "Write Region: " : "Write File: ";
	*path = '\0';
	if (getfname(prompt, path) == 0) {
		if (Argp) {
			putpaw("Writing %s", path);
			write_rgn(path);
			clrecho();
		} else {
			if (Curbuff->fname)
				free(Curbuff->fname);
			Curbuff->fname = strdup(path);
			Curbuff->mtime = 0;	/* this is no longer valid */
			Zfilesave();
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

void Zfileread(void)
{
	if (get_findfile("Read File: "))
		return;
	if (fileread(Fname) > 0) {
		sprintf(PawStr, "Unable to read %s", Fname);
		error(PawStr);
	}
}

/*
Fixup the pathname. 'to' and 'from' cannot overlap.
- if the path starts with a ~, lookup the user in the /etc/passwd file
- add the current directory if not rooted.
- remove the . and .. entries
Returns -1 if the 'from' is a directory
		 1 if the directory portion of a new file is invalid.
		 2 if ~ specifier invalid
		 0 if all is well
NOTE: assumes a valid path (in particular /.. would not work)
*/

int pathfixup(char *to, char *from)
{
	char *start, save, dir[PATHMAX], *p;
	int rc;
	struct stat sbuf;

	do {
		p = strstr(from, "//");
		if (p)
			from = p + 1;
	} while (p);

	start = to;
	*to = '\0';
	if (*from == '~') {
		for (p = dir, ++from; *from && !Psep(*from); ++from, ++p)
			*p = *from;
		*p = '\0';
		if (*dir) {
			struct passwd *pwd = zgetpwnam(dir);
			if (!pwd)
				return 2;
			strcpy(to, pwd->pw_dir);
			free_pwent(pwd);
		} else
			strcpy(to, Me->pw_dir);
		to += strlen(to);

		if (*from && !Psep(*from) && !Psep(*(to - 1)))
			*to++ = PSEP;
	} else if (*from == '$') {
		for (p = dir, ++from; *from && !Psep(*from); ++from, ++p)
			*p = *from;
		*p = '\0';
		p = getenv(dir);
		if (p == NULL)
			return 2;
		strcpy(to, p);
		to += strlen(to);

		if (*from && !Psep(*from) && !Psep(*(to - 1)))
			*to++ = PSEP;
	} else {
		if (VAR(VEXPAND) && !Psep(*from)) {
			/* add the current directory */
			strcpy(to, Cwd);
			to += strlen(to);
			if (!Psep(*(to - 1)))
				*to++ = PSEP;
		}
	}

	if (VAR(VEXPAND)) {
		/* now handle the filename */
		for (; *from; ++from)
			if (*from == '.') {
				if (Psep(*(from + 1)))
					++from;
				else if (*(from + 1) == '.' &&
					 (Psep(*(from + 2)) ||
					  *(from + 2) == '\0')) {
					to -= 2;
					while (to > start && !Psep(*to))
						--to;
					++to;
					if (*(++from + 1))
						++from;
				} else
					*to++ = *from;
			} else if (Psep(*from)) {
				/* strip redundant seperators */
				if (to == start || !Psep(*(to - 1)))
					*to++ = PSEP;
			} else
				*to++ = *from;
		*to = '\0';
	} else
		strcpy(to, from);

	/* validate the filename */
	if (stat(start, &sbuf) == EOF) {
		/* file does not exit - validate the path */
		to = strrchr(start, PSEP);
		if (to) {
			save = *to;
			*to = '\0';
			rc = !isdir(start);
			*to = save;
		} else
			rc = 0;
	} else if (sbuf.st_mode & S_IFDIR)
		rc = -1;
	else
		rc = 0;
	if (strlen(start) >= PATHMAX)
		Dbg("TOO LONG %d '%s'\n", strlen(start), start);
	return rc;
}

Boolean isdir(char *path)
{
	struct stat sbuf;
	return path && stat(path, &sbuf) == 0 && (sbuf.st_mode & S_IFDIR);
}

Boolean isfile(char *path, char *dir, char *fname, Boolean must)
{
	if (!dir || !fname)
		return FALSE;
	strcpy(path, dir);
	if (!Psep(*(path + strlen(path) - 1)))
		strcat(path, "/");
	strcat(path, fname);
	return !must || access(path, 0) == 0;
}

/* Same as getpwnam but handles partial matches. */
static struct passwd *zgetpwnam(char *name)
{
	struct passwd *pwd, *match = NULL;
	int len = strlen(name);

	setpwent();
	while ((pwd = getpwent()))
		if (strncmp(pwd->pw_name, name, len) == 0) {
			if (strcmp(pwd->pw_name, name) == 0) {
				/* full match */
				endpwent();
				return dup_pwent(pwd);
			} else if (!match)
				/* partial match - return first match */
				match = dup_pwent(pwd);
		}
	endpwent();
	return match;
}
