/* zgrep.c - simple standalone grep
 * Copyright (C) 2015-2017 Sean MacLennan
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
#include <fnmatch.h>

/** @addtogroup zedit
 * @{
 */

static void grep_one(char *fname, struct regexp *re,
		     struct buff *inbuff, struct buff *outbuff)
{
	bempty(inbuff);
	if (breadfile(inbuff, fname, NULL))
		return;

	while (re_step(inbuff, re, NULL)) {
		struct mark start;
		unsigned long line = bline(inbuff);

		binstr(outbuff, "%s:%u: ", fname, line);
		zrefresh();

		tobegline(inbuff);
		bmrktopnt(inbuff, &start);
		toendline(inbuff);
		bmove1(inbuff); /* grab NL */
		bcopyrgn(&start, outbuff);
	}
}

static void grepit(char *input, char *files)
{
	struct regexp re;
	DIR *dir;
	struct dirent *ent;
	struct buff *inbuff, *outbuff = Curbuff->buff;

	int rc = re_compile(&re, input, REG_EXTENDED);
	if (rc) {
		re_error(rc, &re, PawStr, COLMAX);
		error("%s", PawStr);
		return;
	}

	dir = opendir(".");
	if (!dir) {
		error("Unable to open directory");
		return;
	}

	inbuff = bcreate();
	if (!inbuff) {
		error("Unable to create tmp file buffer.");
		goto cleanup;
	}

	while ((ent = readdir(dir)) != NULL)
		if (fnmatch(files, ent->d_name, 0) == 0)
			grep_one(ent->d_name, &re, inbuff, outbuff);

cleanup:
	closedir(dir);
	if (inbuff)
		bdelbuff(inbuff);
	re_free(&re);

	message(Curbuff, "Done");
}

void Zgrep(void)
{
	char input[STRMAX + 1], files[STRMAX + 1], *p;
	int rc;
	struct wdo *save = Curwdo;

	getbword(input, STRMAX, bistoken);
	if (getarg("Regex: ", input, STRMAX))
		return;

	if (Curbuff->bmode & CMODE)
		strcpy(files, "*.[ch]");
	else if (Curbuff->bmode & SHMODE) {
		*files = 0;
		if (Curbuff->fname) {
			char *p = strrchr(Curbuff->fname, '.');
			if (p)
				strconcat(files, sizeof(files), "*", p, NULL);
		}
		if (*files == 0)
			strcpy(files, "*.sh");
	} else
		strcpy(files, "*");
	if (getarg("File(s): ", files, STRMAX))
		return;

	p = strrchr(files, '/');
	if (p) {
		*p++ = '\0';
		rc = chdir(files);
		strcpy(files, p);
	} else
		rc = do_chdir(Curbuff);
	if (rc) {
		error("Unable to chdir");
		return;
	}

	if (wuseother(SHELLBUFF)) {
		set_shell_mark();
		grepit(input, files);
		wswitchto(save);
	}
}
/* @} */
