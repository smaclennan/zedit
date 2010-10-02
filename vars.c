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

#define INCLUDE_VARS_STRUCT
#include "z.h"

static char *Readstr(char *str, FILE *fp)
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

	rc = Getplete("Variable: ", NULL, (char **)Vars, VARSIZE, NUMVARS);
	if (rc == -1)
		return;

	if (!Argp || Vars[rc].vtype == STRING) {
		sprintf(pstr, "%s: ", Vars[rc].vname);
		if (Vars[rc].vtype == STRING)
			if (VARSTR(rc))
				strcpy(arg, VARSTR(rc));
			else
				*arg = '\0';
		else
			sprintf(arg, "%d", VAR(rc));
		if (Getarg(pstr, arg, STRMAX))
			return;
		sprintf(pstr, "%s %s", Vars[rc].vname, arg);
		Setavar(pstr, TRUE);
	} else
		Setavar(Vars[rc].vname, TRUE);
}

#ifdef XWINDOWS
int XSetAVar(char *str)
{
	char pstr[STRMAX], *p;
	int var;

	Argp = 0;
	p = strchr(str, ' ');
	if (p)
		*p = '\0';
	for (var = 0; var < NUMVARS; ++var)
		if (strcmp(str, Vars[var].vname) == 0) {
			switch (Vars[var].vtype) {
			case STRING:
			case DECIMAL:
				if (p)
					*p = ' ';
				Setavar(str, TRUE);
				Xflush();
				break;
			case FLAG:
				sprintf(pstr, "%s %d", str, !VAR(var));
				Setavar(pstr, TRUE);
				Xflush();
				break;
			}
			return var;
		}

	return -1;
}
#endif

/* If there is a config.z file, read it! */
/* CExtends, AExtends, TExtends defaults set in comms1.c */
static void ReadConfigFile(char *fname);

void ReadVfile(void)
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
	if (!VARSTR(VMAIL))
		VARSTR(VMAIL) = strdup("mail");
	if (!VARSTR(VMAKE))
		VARSTR(VMAKE) = strdup("make");
	if (!VARSTR(VPRINT))
		VARSTR(VPRINT) = strdup("lp");
	if (!VARSTR(VCEXTS)) {
		VARSTR(VCEXTS) =
			strdup(".c:.h:.cpp:.cc:.cxx:.y:.l:.m:.m4");
		parsem(VARSTR(VCEXTS), CMODE);
	}
	if (!VARSTR(VSEXTS)) {
		VARSTR(VSEXTS) = strdup(".tcl");
		parsem(VARSTR(VSEXTS), TCL);
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
	if (!Isdir(ConfigDir)) {
		ReadConfigFile(ConfigDir);
		ConfigDir = 0;
	}
	for (i = FINDPATHS; i && (i = Findpath(fname, ZCFILE, i, TRUE)); --i)
		ReadConfigFile(fname);
}

static void ReadConfigFile(char *fname)
{
	FILE *fp;
	char buff[STRMAX + 1];

	fp = fopen(fname, "r");
	if (fp) {
		if (Verbose)
			Dbg("Config file %s\n", fname);
		if (Readstr(buff, fp) && strcmp(buff, "#!m4") == 0) {
			fclose(fp);
			sprintf(PawStr, "m4 %s", fname);
			fp = popen(PawStr, "r");
			if (fp == NULL) {
				if (Verbose)
					Dbg("%s failed.\n", PawStr);
				return;
			}
			while (Readstr(buff, fp))
				Setavar(buff, FALSE);
			pclose(fp);
		} else {
			do
				Setavar(buff, FALSE);
			while (Readstr(buff, fp));
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
		if (Strnicmp(ptr, "true", 4) == 0)
			VAR(i) = 1;
		else if (Strnicmp(ptr, "false", 5) == 0)
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
#ifdef XWINDOWS
	if (i == VFONT) {
		if (display == 0)
			return;
		if (LoadFontByName(VARSTR(VFONT)) == 0) {
			sprintf(PawStr, "Unknown font %s.",
				VARSTR(VFONT));
			Error(PawStr);
			return;
		}
	} else
#endif

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
		} else if (i == VMAKE)
			strcpy(mkcmd, VARSTR(i));
		else if (i == VGREP)
			strcpy(grepcmd, VARSTR(i));
		else if (i == VCEXTS)
			parsem(VARSTR(i), CMODE);
		else if (i == VASEXTS)
			parsem(VARSTR(i), ASMMODE);
		else if (i == VASCHAR) {
			/* set current buffer and redisplay */
#if COMMENTBOLD
			Curbuff->comchar = *(char *)VARSTR(VASCHAR);
			Zredisplay();
#endif
		} else if (i == VSEXTS)
			parsem(VARSTR(i), TCL);
		else if (i == VTEXTS)
			parsem(VARSTR(i), TEXT);
}

void Setavar(char *vin, Boolean display)
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
		if (Stricmp(msg, Vars[i].vname) == 0) {
			do_var_match(i, vin);
			if (display) {
				if (i == VTABS || i == VCTABS) {
					Settabsize(Curbuff->bmode);
					Zredisplay();
				} else if (i == VSHOWCWD)
					Newtitle(VAR(i) ? Cwd : NULL);
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
				Echo(msg);
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
			Echo(PawStr);
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
Setmodes is called.
*/
int Tabsize = 8;

int Settabsize(unsigned mode)
{
	int i;

	/* Choose the correct tab size */
	if (mode & CMODE)
		i = VCTABS;
	else
		i = VTABS;
	if (Tmaxcol() != EOF)
		if (VAR(i) > Tmaxcol() - 4)
			VAR(i) = Tmaxcol() - 4;
	if (VAR(i) == 0)
		VAR(i) = 1;
	return Tabsize = VAR(i);
}

void Varval(int code)
{
	switch (Vars[code].vtype) {
	case STRING:
		Binstr(VARSTR(code) ? VARSTR(code) : "NONE");
		break;
	case FLAG:
		Binstr(VAR(code) ? "On" : "Off");
		break;
	case DECIMAL:
		sprintf(PawStr, "%d", VAR(code));
		Binstr(PawStr);
	}
}

void Dline(int trow)
{
	int i;

	if (trow < Tmaxrow() - 2) {
		Tsetpoint(trow, 0);
		Scrnmarks[trow].modf = TRUE;
		for (i = 0; i < Tmaxcol(); ++i)
			Tprntchar('-');
	}
}

/* Save the current variables in the config.z file */
void Zsaveconfig(void)
{
	FILE *fp;
	char fname[PATHMAX];
	int i;

	if (Argp)
		/* use current directory */
		strcpy(fname, ZCFILE);
	else
		/* use home directory */
		sprintf(fname, "%s/%s", Me->pw_dir, ZCFILE);

	if (access(fname, R_OK) == 0 && Ask("Overwrite existing file? ") != YES)
		return;

	fp = fopen(fname, "w");
	if (!fp) {
		sprintf(PawStr, "Unable to create %s: %d", fname, errno);
		Error(PawStr);
		return;
	}

	fprintf(fp, "# String variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == STRING) {
			if (VARSTR(i))
				fprintf(fp, "%-15s %s\n",
					Vars[i].vname, VARSTR(i));
			else
				fprintf(fp, "%-15s 0\n",
					Vars[i].vname);
		}

	fprintf(fp, "\n# Decimal variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == DECIMAL)
			fprintf(fp, "%-15s %d\n", Vars[i].vname, VAR(i));

	fprintf(fp, "\n# Flag variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == FLAG)
			fprintf(fp, "%-15s %s\n", Vars[i].vname,
				VAR(i) ? "True" : "False");

	fclose(fp);
	Echo("Saved.");
}
