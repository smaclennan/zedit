/* getfname.c - get a file name with completion
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
#ifdef SYSV4
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

/* general linked list structure */
struct llist {
	char fname[STRMAX];
	struct llist *prev, *next;
};

static struct llist *Flist;
static Boolean Didmatch;
#define OBJEXT		".o"

static struct llist *add(struct llist **list, char *fname);
static void freelist(struct llist **list);

/* Returns:
 *		0 for ok
 *		ABORT(-1) for abort
 *		> 0 for error
 */
static int getname(char *prompt, char *path, Boolean isdir)
{
	const char mod[3] = { '\t', '/', '~' };
	char tmp[PATHMAX + 1];
	int was[3], rc, i;

	for (i = 0; i < 3; ++i) {
		was[i] = Keys[(int)mod[i]];
		Keys[(int)mod[i]] = ZFNAME;
	}
	rc = getarg(prompt, strcpy(tmp, path), PATHMAX);
	if (rc == 0) {
		rc = pathfixup(path, tmp);
		if (rc == -1)
			rc = isdir ? 0 : 1;
	}
	for (i = 0; i < 3; ++i)
		Keys[(int)mod[i]] = was[i];
	freelist(&Flist);
	if (Didmatch) {
		Zredisplay();
		Didmatch = FALSE;
	}
	return rc;
}

int getfname(char *prompt, char *path)
{
	int rc = getname(prompt, path, FALSE);
	if (rc > 0)
		error("Invalid path.");
	return rc;
}

int getdname(char *prompt, char *path)
{
	int rc = getname(prompt, path, TRUE);
	if (rc > 0)
		error("Invalid dir.");
	return rc;
}

static Boolean isext(char *fname, char *ext)
{
	char *ptr;

	return fname && (ptr = strrchr(fname, '.')) && strcmp(ptr, ext) == 0;
}

static struct llist *fill_list(char *dir)
{
	static char savedir[PATHMAX + 1];
	DIR *dp;
#ifdef SYSV4
	struct dirent *dirp;
#else
	struct direct *dirp;
#endif

	if (Flist && strcmp(dir, savedir) == 0)
		return Flist;
	strcpy(savedir, dir);
	freelist(&Flist);

	dp = opendir(dir);
	if (dp == NULL)
		return Flist;

	while ((dirp = readdir(dp)))
		if (!isext(dirp->d_name, OBJEXT))
			add(&Flist, dirp->d_name);

	closedir(dp);

	return Flist;
}

static struct llist *getfill(char *dir, char **fname, int *len, Boolean *update)
{
	char txt[PATHMAX + 1];

	if (First) {
		bdelete(Curplen);
		First = FALSE;
	}
	getbtxt(txt, PATHMAX);
	if (pathfixup(dir, txt) > 0)
		return NULL;
	*update = strcmp(dir, txt);
	if (*update)
		makepaw(dir, FALSE);
	*fname = lastpart(dir);
	*len = strlen(*fname);

	/* If ExpandPaths not set, may be no directory specified! */
	if (*fname == dir)
		return fill_list("./");

	if (*fname - dir == 1) {
		/* special case for root dir */
		strcpy(txt, *fname);
		strcpy(++*fname, txt);
	}
	*(*fname - 1) = '\0';
	return fill_list(dir);
}

int nmatch(char *s1, char *s2)
{
	int i;

	for (i = 0; tolower(*s1) == tolower(*s2); ++i, ++s1, ++s2)
		;
	return i;
}


static struct llist *add(struct llist **list, char *fname)
{
	struct llist *new, *l;

	new = malloc(sizeof(struct llist));
	if (new) {
		strcpy(new->fname, fname);
		if (*list == NULL || strcmp(fname, (*list)->fname) < 0) {
			new->next = *list;
			if (*list)
				(*list)->prev = new;
			new->prev = NULL;
			*list = new;
		} else {
			for (l = *list;
			     l->next && strcmp(l->next->fname, fname) < 0;
			     l = l->next)
				;
			if (l->next)
				l->next->prev = new;
			new->next = l->next;
			l->next = new;
			new->prev = l;
		}
	}
	return new;
}


static void freelist(struct llist **list)
{
	struct llist *next;

	while (*list) {
		next = (*list)->next;
		free((char *)(*list));
		*list = next;
	}
}

void Zfname(void)
{
	Boolean update;
	struct llist *head, *list;
	char txt[PATHMAX + 1], *fname, *match = NULL;
	char dir[PATHMAX + 1], *p;
	int row, col;
	int len, n = 0, f = 0, rc;
	int did_something

		= 0;

	if (Cmd == '/') {
		if (bpeek() == '/')
			bempty();
		pinsert();
		return;
	} else if (Cmd == '~') {
		bempty();
		pinsert();
		return;
	}

	head = list = getfill(dir, &fname, &len, &update);
	if (!list) {
		tbell();
		return;
	}

	/* Possibly expand name */
	if (*fname)
		while (list && (rc = strncmp(fname, list->fname, len)) >= 0) {
			if (rc == 0) {
				if (match)
					n = f = nmatch(match, list->fname);
				else
					n = strlen(match = list->fname);
			}
			list = list->next;
		}
	if (match) {
		if (n > len) {
			btoend();
			while (len < n && Curplen < Pawlen) {
				binsert(match[len++]);
				++did_something;
			}
			if (len < n)
				tbell();
		}
		if (f == 0 && isdir(getbtxt(txt, PATHMAX)) && Curplen < Pawlen)
			binsert(PSEP);
	} else if (!update)
		tbell();

	if (update || did_something)
		return;

	/* show possible matches */
	list = head; /* reset */
	p = dir + strlen(dir);
	*p++ = PSEP;
	Didmatch = TRUE;
	tgoto(Tstart, 0);
	tprntstr("Choose one of:");
	tcleol();
	row = Tstart + 1; col = 0;
	for (; list; list = list->next)
		if (len == 0 || strncmp(fname, list->fname, len) == 0) {
			tgoto(row, col);
			strcpy(p, list->fname);
			if (strlen(list->fname) > 23)
				list->fname[23] = '\0';
			tprntstr(list->fname);
			if (isdir(dir))
				tputchar(PSEP);
			tcleol();
			col += 25;
			if (col > 72) {
				if (++row < Rowmax - 2)
					col = 0;
				else
					break;
			}
		}
	if (col)
		row++;
	while (row < Rowmax - 2) {
		tgoto(row++, 0);
		tcleol();
	}
}

