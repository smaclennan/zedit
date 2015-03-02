/* zgrep.c - simple standalone grep
 * Copyright (C) 2014 Sean MacLennan
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

#if defined(WIN32)
#define FNM_FLAGS FNM_CASEFOLD
#else
#define FNM_FLAGS 0
#endif

#if 0 // SAM
static void grep_one(char *fname, Byte *ebuf,
			 struct zbuff *inbuff, struct zbuff *outbuff)
{
	zswitchto(inbuff);
	bempty(Bbuff);

	if (zreadfile(fname))
		return;

	while (step(Bbuff, ebuf, NULL)) {
		struct mark *start;
		unsigned long line = bline();

		zswitchto(outbuff);
		snprintf(PawStr, STRMAX, "%s:%ld: ", fname, line);
		binstr(Bbuff, PawStr);
		zswitchto(inbuff);

		tobegline(Bbuff);
		start = zcreatemrk();
		toendline(Bbuff);
		bmove1(Bbuff); /* grab NL */
		bcopyrgn(start, outbuff->buff);
		unmark(start);
	}
}

static void grepit(char *input, char *files)
{
	Byte ebuf[ESIZE];
	DIR *dir;
	struct dirent *ent;
	struct zbuff *inbuff, *outbuff = Curbuff;

	int rc = compile((Byte *)input, ebuf, &ebuf[ESIZE]);
	if (rc) {
		error(regerr(rc));
		return;
	}

	dir = opendir(".");
	if (!dir) {
		error("Unable to open directory");
		return;
	}

	if (!(inbuff = bcreate())) {
		error("Unable to create tmp file buffer.");
		goto cleanup;
	}

	while ((ent = readdir(dir)) != NULL)
		if (fnmatch(files, ent->d_name, FNM_FLAGS) == 0)
			grep_one(ent->d_name, ebuf, inbuff, outbuff);

cleanup:
	closedir(dir);
	zswitchto(outbuff);
	if (inbuff)
		bdelbuff(inbuff);
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
	else if (Curbuff->bmode & SHMODE)
		strcpy(files, "*.sh");
	else
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
#else
void Zgrep(void)
{
	error("disable");
}
#endif
