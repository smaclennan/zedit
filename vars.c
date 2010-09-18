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

Proc Zsetavar()
{
	char pstr[STRMAX], arg[STRMAX];
	int rc;
#if XWINDOWS
	/* If an arg specified, try to start Zvarhelp */
	if (Argp && StartProg("Zvarhelp") == 0)
		return;
#endif

	rc = Getplete("Variable: ", NULL, (char **)Vars, VARSIZE, NUMVARS);
	if (rc == -1)
		return;

	if (!Argp || Vars[rc].vtype == STRING) {
		sprintf(pstr, "%s: ", Vars[rc].vname);
		if (Vars[rc].vtype == STRING)
			if (Vars[rc].val)
				strcpy(arg, (char *)Vars[rc].val);
			else
				*arg = '\0';
		else
			sprintf(arg, "%d", Vars[rc].val);
		if (Getarg(pstr, arg, STRMAX))
			return;
		sprintf(pstr, "%s %s", Vars[rc].vname, arg);
		Setavar(pstr, TRUE);
	} else
		Setavar(Vars[rc].vname, TRUE);
}

#if XWINDOWS
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
				sprintf(pstr, "%s %d", str, !Vars[var].val);
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

void ReadVfile()
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
	if (!Vars[VMAIL].val)
		Vars[VMAIL].val = (Word)strdup("mail");
	if (!Vars[VMAKE].val)
		Vars[VMAKE].val = (Word)strdup("make");
	if (!Vars[VPRINT].val)
		Vars[VPRINT].val = (Word)strdup("lp");
	if (!Vars[VCEXTS].val) {
		Vars[VCEXTS].val =
			(Word)strdup(".c:.h:.cpp:.cc:.cxx:.y:.l:.m:.m4");
		parsem((char *)Vars[VCEXTS].val, CMODE);
	}
	if (!Vars[VSEXTS].val) {
		Vars[VSEXTS].val = (Word)strdup(".tcl");
		parsem((char *)Vars[VSEXTS].val, TCL);
	}
	if (!Vars[VTEXTS].val) {
		Vars[VTEXTS].val = (Word)strdup(".DOC:.doc:.tex:.txt:.d");
		parsem((char *)Vars[VTEXTS].val, TEXT);
	}
	if (!Vars[VASEXTS].val) {
		Vars[VASEXTS].val = (Word)strdup(".s:.asm");
		parsem((char *)Vars[VASEXTS].val, ASMMODE);
	}
	if (!Vars[VASCHAR].val)
		Vars[VASCHAR].val = (Word)strdup(";");


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
		if (Vars[i].val)
			free((char *)Vars[i].val);
		Vars[i].val = (Word)strdup(ptr);
	} else if (Vars[i].vtype == FLAG) {
		if (Strnicmp(ptr, "true", 4) == 0)
			Vars[i].val = 1;
		else if (Strnicmp(ptr, "false", 5) == 0)
			Vars[i].val = 0;
		else
			Vars[i].val = (int)strtol(ptr, NULL, 0);
	} else
		Vars[i].val = (int)strtol(ptr, NULL, 0);
}

static void do_var_match(int i, char *vin)
{
	char *ptr;

	if (Verbose > 1)
		Dbg("ok\n");
	if (Argp && Vars[i].vtype != STRING)
		Vars[i].val = Arg;
	else {
		for (ptr = vin; *ptr && !isspace(*ptr); ++ptr)
			;
		while (isspace(*ptr))
			++ptr;
		if (*ptr)
			setit(i, ptr);
	}
#if XWINDOWS
	if (i == VFONT) {
		if (display == 0)
			return;
		if (LoadFontByName(Vars[VFONT].val) == 0) {
			sprintf(PawStr, "Unknown font %s.",
				Vars[VFONT].val);
			Error(PawStr);
			return;
		}
	} else
#endif

		/* This block handles the Wordprocessing variables */
		if (i == VFILLWIDTH || i == VMARGIN) {
			/* These values MUST be positive */
			Vars[i].val = abs(Vars[i].val);
			/* Fillwidth must be > 0 */
			if (Vars[VFILLWIDTH].val == 0)
				Vars[VFILLWIDTH].val = 1;
			/* Fillwidth must be greater than Margin */
			if (Vars[VFILLWIDTH].val <= Vars[VMARGIN].val) {
				if (i == VMARGIN)
					Vars[VMARGIN].val =
						Vars[VFILLWIDTH].val - 1;
				else
					Vars[VFILLWIDTH].val =
						Vars[VMARGIN].val + 1;
				}
		} else if (i == VMAKE)
			strcpy(mkcmd, (char *)Vars[i].val);
		else if (i == VGREP)
			strcpy(grepcmd, (char *)Vars[i].val);
		else if (i == VCEXTS)
			parsem((char *)Vars[i].val, CMODE);
		else if (i == VASEXTS)
			parsem((char *)Vars[i].val, ASMMODE);
		else if (i == VASCHAR) {
			/* set current buffer and redisplay */
#if COMMENTBOLD
			Curbuff->comchar = *(char *)Vars[VASCHAR].val;
			Zredisplay();
#endif
		} else if (i == VSEXTS)
			parsem((char *)Vars[i].val, TCL);
		else if (i == VTEXTS)
			parsem((char *)Vars[i].val, TEXT);
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
					Newtitle(Vars[i].val ? Cwd : NULL);
				if (Vars[i].vtype == STRING) {
					if (Vars[i].val)
						sprintf(msg, "%s = %s",
							Vars[i].vname,
							(char *)Vars[i].val);
					else
						sprintf(msg, "%s = NONE",
							Vars[i].vname);
				} else
					sprintf(msg, "%s = %d",
						Vars[i].vname, Vars[i].val);
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
	Vars[i].val = abs(Vars[i].val);
	if (Tmaxcol() != EOF)
		if (Vars[i].val > Tmaxcol() - 4)
			Vars[i].val = Tmaxcol() - 4;
	if (Vars[i].val == 0)
		Vars[i].val = 1;
	return Tabsize = Vars[i].val;
}

void Varval(struct avar *var)
{
	switch ((int)var->vtype) {
	case STRING:
		Binstr(var->val ? (char *)var->val : "NONE");
		break;
	case FLAG:
		Binstr(var->val ? "On" : "Off");
		break;
	case DECIMAL:
		sprintf(PawStr, "%d", var->val);
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
Proc Zsaveconfig()
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
			if (Vars[i].val)
				fprintf(fp, "%-15s %s\n",
					Vars[i].vname, (char *)Vars[i].val);
			else
				fprintf(fp, "%-15s 0\n",
					Vars[i].vname);
		}

	fprintf(fp, "\n# Decimal variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == DECIMAL)
			fprintf(fp, "%-15s %d\n", Vars[i].vname, Vars[i].val);

	fprintf(fp, "\n# Flag variables:\n");
	for (i = 0; i < NUMVARS; ++i)
		if (Vars[i].vtype == FLAG)
			fprintf(fp, "%-15s %s\n", Vars[i].vname,
				Vars[i].val ? "True" : "False");

	fclose(fp);
	Echo("Saved.");
}
