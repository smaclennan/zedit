/* tags.c - tag file commands
 * Copyright (C) 2013 Sean MacLennan
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

static char Tagfile[PATHMAX];

/* SAM Currently works for functions only */
/* Leaves the point at the char offset */
#define TAG_REGEX "[^a-zA-Z0-9_]%s(.*[0-9][0-9]*,"

static int get_tagfile(void)
{
	if (Argp) {
		Arg = 0;
		return getfname("Tagfile: ", Tagfile);
	}

	if (access(Tagfile, F_OK) == 0)
		return 0;

	if (Curbuff->fname) {
		char *p;
		strcpy(Tagfile, Curbuff->fname);
		p = strrchr(Tagfile, '/');
		if (p) {
			++p;
			strcpy(p, "TAGS");
			if (access(Tagfile, F_OK) == 0)
				return 0;
		}
	} else {
		zgetcwd(Tagfile, sizeof(Tagfile) - 5);
		strcat(Tagfile, "TAGS");
		if (access(Tagfile, F_OK) == 0)
			return 0;
	}

	*Tagfile = '\0';
	return getfname("Tagfile: ", Tagfile);
}

static bool tagfile_modified(struct buff *buff)
{
	struct stat sbuf;

	if (strcmp(buff->fname, Tagfile))
		return true;

	if (stat(Tagfile, &sbuf) || sbuf.st_mtime > buff->mtime)
		return true;

	return false;
}

/* May change Curbuff */
static struct buff *read_tagfile(void)
{
	struct buff *buff;

	if (get_tagfile())
		return NULL;

	/* Check if we can reuse an existing tag buffer */
	buff = cfindbuff(TAGBUFF);
	if (buff && tagfile_modified(buff) == false)
		return buff;

	buff = cmakebuff(TAGBUFF, Tagfile);
	if (!buff)
		return NULL;

	if (zreadfile(Tagfile))
		return NULL;

	return buff;
}

static bool find_tag(char *word)
{
	char path[PATHMAX], regstr[STRMAX], *p;
	Byte ebuf[ESIZE];
	struct buff *buff, *save = Curbuff;
	int offset;

	buff = read_tagfile();
	bswitchto(save);
	if (!buff)
		return false;

	snprintf(regstr, sizeof(regstr), TAG_REGEX, word);
	if (compile((Byte *)regstr, ebuf, &ebuf[ESIZE])) {
		putpaw("regcomp failed");
		return false;
	}

	bswitchto(buff);
	btostart();
	if (!step(ebuf))
		goto failed;

	offset = batoi();

	if (!bcrsearch(014)) /* C-L */
		goto failed;
	bmove(2);

	strcpy(path, Tagfile);
	p = strrchr(path, '/');
	if (p)
		++p;
	else
		p = path + strlen(path);
	getbword(p, sizeof(path), bistoken);

	bswitchto(save);
	set_bookmark(word);

	if (findfile(path)) {
		bgoto_char(offset);
		redisplay();
		return true;
	}

failed:
	putpaw("No match.");
	bswitchto(save);
	return false;
}

void Ztag(void)
{
	char tag[STRMAX];

	*tag = '\0';
	if (getarg("Tag: ", tag, sizeof(tag)) == 0)
		find_tag(tag);
}

void Ztag_word(void)
{
	char tag[STRMAX];

	getbword(tag, sizeof(tag), bisword);
	find_tag(tag);
}
