/* vars.c - commands for Zedit variables
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

static void setavar(const char *vin, bool display);

void Zset_variable(void)
{
	char pstr[STRMAX], arg[STRMAX];
	int rc = getplete("Variable: ", NULL, (char **)Vars,
			  sizeof(struct avar), NUMVARS);
	if (rc == -1)
		return;

	if (!Argp || Vars[rc].vtype == V_STRING) {
		sprintf(pstr, "%s: ", Vars[rc].vname);
		if (Vars[rc].vtype == V_STRING)
			if (VARSTR(rc))
				strcpy(arg, VARSTR(rc));
			else
				*arg = '\0';
		else
			sprintf(arg, "%d", VAR(rc));
		if (getarg(pstr, arg, STRMAX))
			return;
		sprintf(pstr, "%s %s", Vars[rc].vname, arg);
		setavar(pstr, true);
	} else
		setavar(Vars[rc].vname, true);
}

static void bindone(char *line)
{
	char cmd[STRMAX];
	int key, i;

	if (sscanf(line, "bind %o %s", &key, cmd) != 2) {
		Dbg("Bad bind line %s\n", line);
		return;
	}
	if (key < 0 || key >= NUMKEYS) {
		Dbg("Invalid key %d\n", key);
		return;
	}

	for (i = 0; i < NUMFUNCS; ++i)
		if (strcmp(Cnames[i].name, cmd) == 0) {
			Keys[key] = Cnames[i].fnum;
			return;
		}

	Dbg("Invalid cmd %s\n", cmd);
}

/* If there is a config.z file, read it! */
void readvfile(const char *fname)
{
	char line[STRMAX + 1];
	FILE *fp = fopen(fname, "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp)) {
			char *p = strchr(line, '\n');
			if (p)
				*p = '\0';
			if (strncmp(line, "bind", 4) == 0)
				bindone(line);
			else
				setavar(line, false);
		}
		fclose(fp);
	}
}

static void setit(int i, const char *ptr)
{
	switch (Vars[i].vtype) {
	case V_STRING:
		if (strcmp(VARSTR(i), ptr))
			VARSTR(i) = strdup(ptr);
		break;
	case V_FLAG:
		if (strncasecmp(ptr, "true", 4) == 0)
			VAR(i) = 1;
		else if (strncasecmp(ptr, "false", 5) == 0)
			VAR(i) = 0;
		else
			VAR(i) = strtol(ptr, NULL, 0);
		break;
	case V_DECIMAL:
		VAR(i) = strtol(ptr, NULL, 0);
	}
}

static void do_var_match(int i, const char *vin)
{
	const char *ptr;

	if (Argp && Vars[i].vtype != V_STRING)
		VAR(i) = Arg;
	else {
		for (ptr = vin; *ptr && !isspace(*ptr); ++ptr)
			;
		while (isspace(*ptr))
			++ptr;
		if (*ptr)
			setit(i, ptr);
	}

	/* Fillwidth must be > 0 */
	if (VAR(VFILLWIDTH) == 0)
		VAR(VFILLWIDTH) = 1;
}

static void setavar(const char *vin, bool display)
{
	char *ptr, msg[STRMAX + 1];
	int i = 0;

	strcpy(msg, vin);
	ptr = strchr(msg, ' ');
	if (ptr)
		*ptr = '\0';
	ptr = strchr(msg, '\t');
	if (ptr)
		*ptr = '\0';

	for (i = 0; i < NUMVARS; ++i)
		if (strcasecmp(msg, Vars[i].vname) == 0) {
			do_var_match(i, vin);
			if (display) {
				if (i == VTABS || i == VCTABS) {
					settabsize(Curbuff->bmode);
					redisplay();
				}
				if (Vars[i].vtype == V_STRING) {
					if (VARSTR(i))
						putpaw("%s = %s",
						       Vars[i].vname,
						       VARSTR(i));
					else
						putpaw("%s = NONE",
						       Vars[i].vname);
				} else
					putpaw("%s = %d",
						Vars[i].vname, VAR(i));
				if (i == VCOMMENTS)
					redisplay();
			}
			break;
		}
	if (i == NUMVARS) {
		if (display)
			putpaw("Variable '%s' Not Found", vin);
	}
	Arg = 0;
}

/* Set Tabsize variable. Tabs must be >= 2 and <= 8. */
int Tabsize = 8;

int settabsize(unsigned mode)
{
	int i;

	/* Choose the correct tab size */
	if (mode & CMODE)
		i = VCTABS;
	else if (mode & SHMODE)
		i = VSHTABS;
	else
		i = VTABS;
	if (VAR(i) <= 0)
		VAR(i) = 2;
	else if (VAR(i) > 8)
		VAR(i) = 8;
	return Tabsize = VAR(i);
}

void Zshow_config(void)
{
	int i;
	char line[STRMAX * 2];
	struct buff *tbuff = cmakebuff(CONFBUFF, NULL);
	if (!tbuff)
		return;

	bempty();
	for (i = 0; i < NUMVARS; ++i) {
		if (Vars[i].vtype == V_STRING) {
			if (VARSTR(i))
				snprintf(line, sizeof(line), "%-15s %s\n",
					 Vars[i].vname, VARSTR(i));
			else
				snprintf(line, sizeof(line), "#%-15s\n",
					 Vars[i].vname);
		} else if (Vars[i].vtype == V_DECIMAL)
			snprintf(line, sizeof(line), "%-15s %d\n",
				 Vars[i].vname, VAR(i));
		else if (Vars[i].vtype == V_FLAG)
			snprintf(line, sizeof(line), "%-15s %s\n",
				 Vars[i].vname,
				 VAR(i) ? "true" : "false");

		binstr(line);
	}

	tbuff->bmodf = false;
	btostart();
	cswitchto(tbuff);
}
