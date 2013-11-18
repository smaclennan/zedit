/* help.c - Zedit help command
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
#include "keys.h"

static char *Htype[] = {
	"Bindings",
	"Buffer/Window",
	"Copy/Delete",
	"Cursor",
	"Display",
	"File",
	"Help/Status",
	"Mode",
	"Other",
	"Search/Replace",
	"Shell",
	"Special",
};
#define HTYPES	(sizeof(Htype) / sizeof(char *))

static void dump_bindings(int fnum)
{
	int k, found = 0;
	char buff[BUFSIZ];

	binstr("\nBinding(s): ");

	for (k = 0; k < NUMKEYS; ++k)
		if (Keys[k] == fnum)
			if (notdup_key(k)) {
				if (found)
					binstr(",  ");
				else
					found = true;
				binstr(dispkey(k, buff));
			}

	if (!found)
		binstr("Unbound");
}

void Zhelp_function(void)
{
	char *p;
	int rc = getplete("Function: ", NULL, (char **)Cnames,
			  CNAMESIZE, NUMFUNCS);
	if (rc == -1)
		return;

	wuseother(HELPBUFF);

	binstr(Cnames[rc].name);
	binstr("\n\n");

	for (p = Cnames[rc].doc; *p; ++p)
		if (*p == ' ') {
			Cmd = *p;
			Zfill_check();
		} else
			binsert(*p);
	binsert('\n');

	if (Cnames[rc].fnum != ZNOTIMPL &&
	    Cnames[rc].fnum != ZINSERT)
		dump_bindings(Cnames[rc].fnum);

	btostart();
}

void Zhelp_group(void)
{
	char line[80];
	int i, j, n;
	int rc = getplete("Group: ", NULL, (char **)Htype,
			  sizeof(char *), HTYPES);
	if (rc == -1)
		return;

	wuseother(HELPBUFF);

	binstr(Htype[rc]);
	binstr("\n\n");

	for (i = j = n = 0; i < NUMFUNCS; ++i)
		if (Cnames[i].htype == rc) {
			n += sprintf(line + n, "%-24s", Cnames[i].name);
			if (++j == 3) {
				binstr(line);
				binsert('\n');
				j = n = 0;
			}
		}

	if (j)
		binstr(line);
	binsert('\n');

	btostart();
}
