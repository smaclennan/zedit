/* getfname.c - get a file name with completion
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
#ifndef WIN32
#ifdef HAVE_DIRECT
#include <sys/dir.h>
#define dirent direct
#else
#include <dirent.h>
#endif
#endif

#define PSEP		'/'
#define Psep(c)		(c == PSEP)


/* general linked list structure */
struct llist {
	char fname[STRMAX];
	struct llist *prev, *next;
};

static struct llist *Flist;
static bool Didmatch;
#if defined(WIN32) || defined(DOS)
#define OBJEXT ".obj"
#else
#define OBJEXT ".o"
#endif

static struct llist *add(struct llist **list, char *fname);
static void freelist(struct llist **list);

/* Returns:
 *		0 for ok
 *		ABORT(-1) for abort
 *		> 0 for error
 */
int getfname(const char *prompt, char *path)
{
	const char mod[3] = { '\t', '/', '~' };
	char tmp[PATHMAX + 1];
	int was[3], rc, i;

	for (i = 0; i < 3; ++i) {
		was[i] = Keys[(int)mod[i]];
		Keys[(int)mod[i]] = ZFNAME;
	}
	rc = _getarg(prompt, strcpy(tmp, path), PATHMAX, false);
	if (rc == 0) {
		rc = pathfixup(path, tmp);
		if (rc == -1)
			rc = 1;
		if (rc)
			error("Invalid path.");
	}
	for (i = 0; i < 3; ++i)
		Keys[(int)mod[i]] = was[i];
	freelist(&Flist);
	if (Didmatch) {
		redisplay();
		Didmatch = false;
	}
	return rc;
}

static bool isext(char *fname, const char *ext)
{
	char *ptr;
#ifdef DOS
	return fname && (ptr = strrchr(fname, '.')) != NULL && 
		stricmp(ptr, ext) == 0;
#else
	return fname && (ptr = strrchr(fname, '.')) && strcmp(ptr, ext) == 0;
#endif
}

#if defined(DOS)
#define fnamelower strlwr
#else
#define fnamelower(f) f
#endif

static struct llist *fill_list(const char *dir)
{
	static char savedir[PATHMAX + 1];
	DIR *dp;
	struct dirent *dirp;

	if (Flist && strcmp(dir, savedir) == 0)
		return Flist;
	strcpy(savedir, dir);
	freelist(&Flist);

	dp = opendir(dir);
	if (dp == NULL)
		return Flist;

	while ((dirp = readdir(dp)) != NULL)
		if (!isext(dirp->d_name, OBJEXT))
			add(&Flist, fnamelower(dirp->d_name));

	closedir(dp);

	return Flist;
}

static struct llist *getfill(char *dir, char **fname, int *len, bool *update)
{
	char txt[PATHMAX + 1];

	if (First) {
		bdelete(Curplen);
		First = false;
	}
	getbtxt(txt, PATHMAX);
	if (pathfixup(dir, txt) > 0)
		return NULL;
	*update = strcmp(dir, txt);
	if (*update)
		makepaw(dir, false);
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
	struct llist *newl, *l;

	newl = (struct llist *)malloc(sizeof(struct llist));
	if (newl) {
		strcpy(newl->fname, fname);
		if (*list == NULL || strcmp(fname, (*list)->fname) < 0) {
			newl->next = *list;
			if (*list)
				(*list)->prev = newl;
			newl->prev = NULL;
			*list = newl;
		} else {
			for (l = *list;
			     l->next && strcmp(l->next->fname, fname) < 0;
			     l = l->next)
				;
			if (l->next)
				l->next->prev = newl;
			newl->next = l->next;
			l->next = newl;
			newl->prev = l;
		}
	}
	return newl;
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

static bool isdir(char *path)
{
	struct stat sbuf;
	return path && stat(path, &sbuf) == 0 && (sbuf.st_mode & S_IFDIR);
}

void Zfname(void)
{
	bool update;
	struct llist *head, *list;
	char txt[PATHMAX + 1], *fname, *match = NULL;
	char dir[PATHMAX + 1], *p;
	int row, col;
	int len, n = 0, f = 0, rc;
	int did_something = 0;

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
	Didmatch = true;
	t_goto(0, 0);
	tprntstr("Choose one of:");
	tcleol();
	row = 1; col = 0;
	for (; list; list = list->next)
		if (len == 0 || strncmp(fname, list->fname, len) == 0) {
			t_goto(row, col);
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
		t_goto(row++, 0);
		tcleol();
	}
}

#if !defined(WIN32) && !defined(DOS)
#include <pwd.h>

/* Get users directory - handles partial matches. */
static bool zgetpwdir(char *name, char *to)
{
	struct passwd *pwd;
	bool match = false;
	int len = strlen(name);

	setpwent();
	while ((pwd = getpwent()))
		if (strncmp(pwd->pw_name, name, len) == 0) {
			if (strcmp(pwd->pw_name, name) == 0) {
				/* full match */
				endpwent();
				strcpy(to, pwd->pw_dir);
				return true;
			} else if (!match) {
				/* partial match - return first match */
				strcpy(to, pwd->pw_dir);
				match = true;
			}
		}
	endpwent();
	return match;
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
			if (!zgetpwdir(dir, to))
				return 2;
		} else
			strcpy(to, Home);
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
		if (!Psep(*from)) {
			/* add the current directory */
			zgetcwd(to, PATHMAX);
			to += strlen(to);
			*to++ = PSEP;
		}
	}

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
#endif /* WIN32 */
