/* vars.c - commands for Zedit variables
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
#include "vars-array.h"

static void setavar(char *vin, Boolean display);

static char *readstr(char *str, FILE *fp)
{
	if (fgets(str, STRMAX, fp)) {
		char *ptr = strchr(str, '\n');
		if (ptr)
			*ptr = '\0';
		return str;
	}
	return NULL;
}

void Zsetavar(void)
{
	char pstr[STRMAX], arg[STRMAX];
	int rc;

	rc = getplete("Variable: ", NULL, (char **)Vars, VARSIZE, NUMVARS);
	if (rc == -1)
		return;

	if (!Argp || Vars[rc].vtype == STRING) {
		sprintf(pstr, "%s: ", Vars[rc].vname);
		if (Vars[rc].vtype == STRING)
			if (VARSTR(rc))
				strcpy(arg, VARSTR(rc));
			else if (rc == VMAKE)
				strcpy(arg, MAKE_CMD);
			else if (rc == VGREP)
				strcpy(arg, GREP_CMD);
			else
				*arg = '\0';
		else
			sprintf(arg, "%d", VAR(rc));
		if (getarg(pstr, arg, STRMAX))
			return;
		sprintf(pstr, "%s %s", Vars[rc].vname, arg);
		setavar(pstr, TRUE);
	} else
		setavar(Vars[rc].vname, TRUE);
}

/* If there is a config.z file, read it! */
/* CExtends, AExtends, TExtends defaults set in comms1.c */
static void readconfigfile(char *fname);

void readvfile(void)
{
	char fname[PATHMAX + 1];
	int i;

#if DBG
	if (NUMVARS != VARNUM) {
		/* haven't setup term stuff yet */
		printf("\7Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARNUM);
		getchar();
	}
#endif
	if (!VARSTR(VCEXTS)) {
		VARSTR(VCEXTS) =
			strdup(".c:.h:.cpp:.cc:.cxx:.y:.l:.m:.m4");
		parsem(VARSTR(VCEXTS), CMODE);
	}
	if (!VARSTR(VSEXTS)) {
		VARSTR(VSEXTS) = strdup(".sh:.csh");
		parsem(VARSTR(VSEXTS), SHMODE);
	}
	if (!VARSTR(VTEXTS)) {
		VARSTR(VTEXTS) = strdup(".DOC:.doc:.tex:.txt:.d");
		parsem(VARSTR(VTEXTS), TEXT);
	}
	if (!VARSTR(VASEXTS)) {
		VARSTR(VASEXTS) = strdup(".s:.asm");
		parsem(VARSTR(VASEXTS), ASMMODE);
	}
	if (!VARSTR(VASCHAR))
		VARSTR(VASCHAR) = strdup(";");
	if (!VARSTR(VDATESTR))
		VARSTR(VDATESTR) = strdup("%c");


	/* If ConfigDir is really a file, read the file and set to 0. */
	if (ConfigDir && !isdir(ConfigDir)) {
		readconfigfile(ConfigDir);
		ConfigDir = NULL;
	}
	for (i = FINDPATHS; i && (i = findpath(fname, ZCFILE, i, TRUE)); --i)
		readconfigfile(fname);
}

static void readconfigfile(char *fname)
{
	FILE *fp;
	char buff[STRMAX + 1];

	fp = fopen(fname, "r");
	if (fp) {
		if (Verbose)
			Dbg("Config file %s\n", fname);
		if (readstr(buff, fp) && strcmp(buff, "#!m4") == 0) {
			fclose(fp);
			sprintf(PawStr, "m4 %s", fname);
			fp = popen(PawStr, "r");
			if (fp == NULL) {
				if (Verbose)
					Dbg("%s failed.\n", PawStr);
				return;
			}
			while (readstr(buff, fp))
				setavar(buff, FALSE);
			pclose(fp);
		} else {
			do
				setavar(buff, FALSE);
			while (readstr(buff, fp));
			fclose(fp);
		}
	}
}

static void setit(int i, char *ptr)
{
	if (Vars[i].vtype == STRING) {
		if (VARSTR(i))
			free(VARSTR(i));
		VARSTR(i) = strdup(ptr);
	} else if (Vars[i].vtype == FLAG) {
		if (strncasecmp(ptr, "true", 4) == 0)
			VAR(i) = 1;
		else if (strncasecmp(ptr, "false", 5) == 0)
			VAR(i) = 0;
		else
			VAR(i) = strtol(ptr, NULL, 0);
	} else
		VAR(i) = strtol(ptr, NULL, 0);
}

static void do_var_match(int i, char *vin)
{
	char *ptr;

	if (Verbose > 1)
		Dbg("ok\n");
	if (Argp && Vars[i].vtype != STRING)
		VAR(i) = Arg;
	else {
		for (ptr = vin; *ptr && !isspace(*ptr); ++ptr)
			;
		while (isspace(*ptr))
			++ptr;
		if (*ptr)
			setit(i, ptr);
	}

	/* This block handles the Wordprocessing variables */
	if (i == VFILLWIDTH || i == VMARGIN) {
		/* Fillwidth must be > 0 */
		if (VAR(VFILLWIDTH) == 0)
			VAR(VFILLWIDTH) = 1;
		/* Fillwidth must be greater than Margin */
		if (VAR(VFILLWIDTH) <= VAR(VMARGIN)) {
			if (i == VMARGIN)
				VAR(VMARGIN) =
					VAR(VFILLWIDTH) - 1;
			else
				VAR(VFILLWIDTH) =
					VAR(VMARGIN) + 1;
		}
	} else if (i == VCEXTS)
		parsem(VARSTR(i), CMODE);
	else if (i == VASEXTS)
		parsem(VARSTR(i), ASMMODE);
	else if (i == VASCHAR) {
		/* set current buffer and redisplay */
#if COMMENTBOLD
		if (!Initializing) {
			Curbuff->comchar = *VARSTR(VASCHAR);
			Zredisplay();
		}
#endif
	} else if (i == VSEXTS)
		parsem(VARSTR(i), SHMODE);
	else if (i == VTEXTS)
		parsem(VARSTR(i), TEXT);
}

static void setavar(char *vin, Boolean display)
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
	if (Verbose > 1)
		Dbg("SetAVar '%s' (%s) ", msg, vin);

	for (i = 0; i < NUMVARS; ++i)
		if (strcasecmp(msg, Vars[i].vname) == 0) {
			do_var_match(i, vin);
			if (display) {
				if (i == VTABS || i == VCTABS) {
					settabsize(Curbuff->bmode);
					Zredisplay();
				}
				if (Vars[i].vtype == STRING) {
					if (VARSTR(i))
						sprintf(msg, "%s = %s",
							Vars[i].vname,
							VARSTR(i));
					else
						sprintf(msg, "%s = NONE",
							Vars[i].vname);
				} else
					sprintf(msg, "%s = %d",
						Vars[i].vname, VAR(i));
				echo(msg);
				if (i == VLINES)
					Curwdo->modeflags = INVALID;
			}
			break;
		}
	if (i == NUMVARS) {
		if (Verbose > 1)
			Dbg("no match\n");
		if (display) {
			sprintf(PawStr, "Variable '%s' Not Found", vin);
			echo(PawStr);
		}
	}
	Arg = 0;
}

/*
Set Tabsize variable. Uses Ctabs if the buffer is in C mode, else Tabs.

Tabs must be > 0 and <= screen width - 4.
The -4 takes into account borders and a fudge factors.
NOTE: During setup, screen width is undefined.

This routine is called when VTABS or VCTABS is changed or
setmodes is called.
*/
int Tabsize = 8;

int settabsize(unsigned mode)
{
	int i;

	/* Choose the correct tab size */
	if (mode & CMODE)
		i = VCTABS;
	else
		i = VTABS;
	if (tmaxcol() != EOF)
		if (VAR(i) > tmaxcol() - 4)
			VAR(i) = tmaxcol() - 4;
	if (VAR(i) == 0)
		VAR(i) = 1;
	return Tabsize = VAR(i);
}

void varval(int code)
{
	switch (Vars[code].vtype) {
	case STRING:
		binstr(VARSTR(code) ? VARSTR(code) : "NONE");
		break;
	case FLAG:
		binstr(VAR(code) ? "On" : "Off");
		break;
	case DECIMAL:
		sprintf(PawStr, "%d", VAR(code));
		binstr(PawStr);
	}
}

/* Show the current variables in a buffer suitable for writing to file */
void Zshowconfig(void)
{
	int i;
	char line[STRMAX * 2];
	struct buff *tbuff = cmakebuff(CONFBUFF, NULL);
	if (!tbuff)
		return;

	bempty();
	binstr("# String variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == STRING) {
			if (VARSTR(i))
				snprintf(line, sizeof(line), "%-15s %s\n",
					 Vars[i].vname, VARSTR(i));
			else
				snprintf(line, sizeof(line), "%-15s 0\n",
					 Vars[i].vname);
			binstr(line);
		}

	binstr("\n# Decimal variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == DECIMAL) {
			snprintf(line, sizeof(line), "%-15s %d\n", Vars[i].vname, VAR(i));
			binstr(line);
		}

	binstr("\n# Flag variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == FLAG) {
			snprintf(line, sizeof(line), "%-15s %s\n", Vars[i].vname,
				VAR(i) ? "True" : "False");
			binstr(line);
		}

	tbuff->bmodf = FALSE;
	btostart();
	cswitchto(tbuff);
}

void vfini(void)
{
	int i;

	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == STRING)
			if (VARSTR(i))
				free(VARSTR(i));

	free_extensions();
}
