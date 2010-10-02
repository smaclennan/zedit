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

static struct llist *Flist;
static Boolean Didmatch;


static int getname(char*, char*, Boolean);

int Getfname(char *prompt, char *path)
{
	int rc = getname(prompt, path, FALSE);
	if (rc > 0)
		Error("Invalid path.");
	return rc;
}

int Getdname(char *prompt, char *path)
{
	int rc = getname(prompt, path, TRUE);
	if (rc > 0)
		Error("Invalid dir.");
	return rc;
}

/* Returns:
 *		0 for ok
 *		ABORT(-1) for abort
 *		> 0 for error
 */
static int getname(char *prompt, char *path, Boolean isdir)
{
	char tmp[PATHMAX + 1];
	int tab, rc;

	tab = Keys['\t'];
	Keys['\t'] = ZFNAME;
	rc = Getarg(prompt, strcpy(tmp, path), PATHMAX);
	if (rc == 0) {
		rc = pathfixup(path, tmp);
		if (rc == -1)
			rc = isdir ? 0 : 1;
	}
	Keys['\t'] = tab;
	Freelist(&Flist);
	if (Didmatch) {
		Zredisplay();
		Didmatch = FALSE;
	}
	return rc;
}

void Zfname(void)
{
	Boolean update;
	struct llist *head, *list;
	char txt[PATHMAX + 1], *fname, *match = NULL;
	char dir[PATHMAX + 1], *p;
	int row, col;
	int len, n = 0, f = 0, rc;
	int did_something = 0;

	head = list = GetFill(dir, &fname, &len, &update);
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
		if (f == 0 && isdir(Getbtxt(txt, PATHMAX)) && Curplen < Pawlen)
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
	Tcleol();
	row = Tstart + 1; col = 0;
	for (; list; list = list->next)
		if (len == 0 || strncmp(fname, list->fname, len) == 0) {
			tgoto(row, col);
			strcpy(p, list->fname);
			if (strlen(list->fname) > 23)
				list->fname[23] = '\0';
			tprntstr(list->fname);
			if (isdir(dir))
				Tputchar(PSEP);
			Tcleol();
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
		Tcleol();
	}
}

struct llist *GetFill(char *dir, char **fname, int *len, Boolean *update)
{
	char txt[PATHMAX + 1];

	if (First) {
		bdelete(Curplen);
		First = FALSE;
	}
	Getbtxt(txt, PATHMAX);
	if (pathfixup(dir, txt) > 0)
		return NULL;
	*update = strcmp(dir, txt);
	if (*update)
		Makepaw(dir, FALSE);
	*fname = Lastpart(dir);
	*len = strlen(*fname);

	/* If ExpandPaths not set, may be no directory specified! */
	if (*fname == dir)
		return Fill_list("./");

	if (*fname - dir == 1) {
		/* special case for root dir */
		strcpy(txt, *fname);
		strcpy(++*fname, txt);
	}
	*(*fname - 1) = '\0';
	return Fill_list(dir);
}

#define OBJEXT		".o"

struct llist *Fill_list(char *dir)
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
	Freelist(&Flist);

	dp = opendir(dir);
	if (dp == NULL)
		return Flist;

	while ((dirp = readdir(dp))) {
#ifdef ULTRIX
		char *fname = dirp->gd_name;
#else
		char *fname = dirp->d_name;
#endif
		if (!Isext(fname, OBJEXT))
			Add(&Flist, fname);
	}

	closedir(dp);

	return Flist;
}

int nmatch(char *s1, char *s2)
{
	int i;

	for (i = 0; Tolower(*s1) == Tolower(*s2); ++i, ++s1, ++s2)
		;
	return i;
}


struct llist *Add(struct llist **list, char *fname)
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


void Freelist(struct llist **list)
{
	struct llist *next;

	while (*list) {
		next = (*list)->next;
		free((char *)(*list));
		*list = next;
	}
}
